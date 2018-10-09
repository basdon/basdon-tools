package grabgxtvalue;
import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.Paths;
import java.util.zip.CRC32;

import static java.lang.String.format;

// ref https://www.grandtheftwiki.com/GXT
public class grabgxtvalue
{
	static boolean debug;
	
	public static void main(String[] args) throws Exception
	{
		final String entryname = System.getProperty("gxtentry");
		final String gxtpath = System.getProperty("gxtpath");
		if (entryname == null) {
			die("specify -Dgxtentry=PARA,CIVI (text / crc int / crc 0xint)");
		}
		if (gxtpath == null) {
			die("specify -Dgxtpath=/path/to/file.gxt");
		}
		debug = System.getProperty("debug") != null;
		
		File gxtfile = new File(gxtpath);
		if (!gxtfile.exists()) {
			gxtfile = Paths.get(gxtpath).toAbsolutePath().toFile();
			if (!gxtfile.exists()) {
				die("cannot resolve file '%s'", gxtpath);
			}
		}
		
		String[] entrynames = entryname.split(",");
		int[] search = new int[entrynames.length];
		for (int i = 0; i < entrynames.length; i++) {
			final String name = entrynames[i];
			try {
				search[i] = Integer.parseInt(name);
			} catch (Exception e) {
				try {
					if (name.startsWith("0x") || name.startsWith("0X")) {
						search[i] = Integer.parseInt(name.substring(2), 16);
					}
				} catch (Exception e2) {
				}
			}
			if (search[i] == 0) {
				final CRC32 crc = new CRC32();
				final byte[] b = name.getBytes(StandardCharsets.US_ASCII);
				crc.update(b, 0, b.length);
				search[i] = (int) (4294967295L - crc.getValue());
			}
			dprintfln("'%s' CRC is %08X", name, search[i]);
		}
		
		byte[] buf = new byte[255];
		int[] stoffsets;
		String[] stnames;
		try (InputStream in = new BufferedInputStream(new FileInputStream(gxtfile))) {
			in.mark(1024 * 1024 * 8);
			in.read(buf, 0, 8); // Version + TABL
			if (buf[4] != 'T' && buf[5] != 'A' && buf[6] != 'B' && buf[7] != 'L') {
				die("not a gxt table");
			}
			in.read(buf, 0, 4);

			int subtables = i32(buf) / 12;
			dprintfln("%d subtables", subtables);
			stoffsets = new int[subtables];
			stnames = new String[subtables];
			for (int i = 0; i < subtables; i++) {
				in.read(buf, 0, 8);
				final String name = s(buf);
				in.read(buf, 0, 4);
				final int offset = i32(buf);
				dprintfln("subtable '%s' offset %08X", name, offset);
				stnames[i] = name;
				stoffsets[i] = offset;
			}

			for (int i = 0; i < subtables; i++) {
				position(in, stoffsets[i]);
				
				int entrybaseoffset = stoffsets[i];
				
				if (!"MAIN".equals(stnames[i])) {
					in.read(buf, 0, 8); // subtable name again
					entrybaseoffset += 8;
				}

				in.read(buf, 0, 4); entrybaseoffset += 4;
				if (buf[0] != 'T' && buf[1] != 'K' &&
					buf[2] != 'E' && buf[3] != 'Y')
				{
					die("not a TKEY");
				}

				in.read(buf, 0, 4); entrybaseoffset += 4;
				int entries = i32(buf) / 8;
				dprintfln("subtable '%s' has %d entries", stnames[i], entries);
				int[] enames = new int[entries];
				int[] eoffsets = new int[entries];
				for (int j = 0; j < entries; j++) {
					in.read(buf, 0, 4);
					final int offset = i32(buf);
					in.read(buf, 0, 4);
					final int name = i32(buf);
					//dprintfln("entry %08X offset %08X", name, offset);
					enames[j] = name;
					eoffsets[j] = offset;
				}
				entrybaseoffset += entries * 8;
				in.read(buf, 0, 4);
				if (buf[0] != 'T' && buf[1] != 'D' &&
					buf[2] != 'A' && buf[3] != 'T')
				{
					die("not at TDAT");
				}
				entrybaseoffset += 8;
				
				for (int j = 0; j < entries; j++) {
					for (int k = 0; k < search.length; k++) {
						if (enames[j] != search[k]) {
							continue;
						}
						position(in, entrybaseoffset + eoffsets[j]);
						in.read(buf, 0, buf.length);
						String value = s(buf);
						System.out.printf(
							"found '%s': %s%n", entrynames[k], value
						);
					}
				}
			}
		}
	}
	
	static void position(InputStream in, int offset) throws IOException
	{
		in.reset();
		while (offset > 0) {
			long skipped = in.skip(offset);
			if (skipped < 1) {
				die("not skipping, need %d more", offset);
			}
			offset -= skipped;
		}
	}
	
	static int i32(byte[] b)
	{
		return (b[0] & 0xFF) | ((b[1] & 0xFF) << 8) |
			((b[2] & 0xFF) << 16) | ((b[3] & 0xFF) << 24);
	}
	
	static String s(byte[] b)
	{
		int len = 0;
		while (b[len] != 0) {
			len++;
		}
		if (len == 0) {
			return "";
		}
		byte[] nb = new byte[len];
		System.arraycopy(b, 0, nb, 0, len);
		return new String(nb, StandardCharsets.US_ASCII);
	}
	
	static void dprintfln(String format, Object...params)
	{
		if (debug) {
			System.out.printf(format + "%n", params);
		}
	}
	
	static void die(String format, Object...params)
	{
		throw new RuntimeException(format(format, params));
	}
}
