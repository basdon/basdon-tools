import java.nio.file.*;
import java.util.*;

import static java.lang.String.*;
import static java.lang.System.out;

public class Carcols
{
	/**
	 * @param args 0: vehicles.ide, 1: carcols.dat
	 */
	public static void main(String args[]) throws Exception
	{
		List<String> vehicles = new ArrayList<>(212);
		HashMap<String, String> carcols = new HashMap<>();
		HashMap<String, Integer> amounts = new HashMap<>();
		List<String> lines = Files.readAllLines(Paths.get(args[0]));
		Iterator<String> iter = lines.iterator();
		while (iter.hasNext() && !"cars".equals(iter.next()));
		while (iter.hasNext()) {
			String line = iter.next();
			if (line.isEmpty() || line.charAt(0) == '#') {
				continue;
			}
			if ("end".equals(line)) {
				break;
			}
			int comma = line.indexOf(',');
			if (comma != -1) {
				while (line.charAt(++comma) == ' ' || line.charAt(comma) == '\t');
				line = line.substring(comma);
				comma = line.indexOf(',');
				if (comma != -1) {
					vehicles.add(line.substring(0, comma));
				}
			}
		}
		if (vehicles.size() != 212) {
			throw new RuntimeException(format(
				"only found %d/212 in vehicles.ide", vehicles.size(), 212));
		}
		lines = Files.readAllLines(Paths.get(args[1]));
		iter = lines.iterator();
		while (iter.hasNext() && !"car".equals(iter.next()));
		while (iter.hasNext()) {
			String line = iter.next().trim();
			if (line.isEmpty() || line.charAt(0) == '#') {
				continue;
			}
			line = line.replaceAll("\t", " ").replaceAll(" +", " ");
			if (line.contains("\t") || line.contains("  ")) {
				throw new RuntimeException("problem");
			}
			String parts[] = line.split(" ");
			String vehicle = parts[0].substring(0, parts[0].length() - 1);
			String[] colcombos = new String[parts.length - 1];
			for (int i = 0; i < colcombos.length; i++) {
				String cc = parts[i + 1];
				if (cc.indexOf(',') != cc.lastIndexOf(',')) {
					// col4
					cc = cc.substring(0, cc.indexOf(',', cc.indexOf(',') + 1));
				}
				colcombos[i] = cc;
			}
			Arrays.sort(colcombos);
			carcols.put(vehicle, String.join(", ", colcombos) + ",");
			amounts.put(vehicle, new Integer(colcombos.length));
			if ("end".equals(line)) {
				while (iter.hasNext() && !"car4".equals(iter.next()));
			}
		}
		List<String> carcolsoutput = new ArrayList<String>(212);
		HashMap<String, Integer> existingpositions = new HashMap<>();
		int offset = 0;
		int totalamount = 0;
		out.println("struct CARCOLDATA carcoldata[VEHICLE_MODEL_TOTAL] = {");
		for (String vehicle : vehicles) {
			int amount = 1;
			String cc = carcols.get(vehicle);
			if (cc == null) {
				cc = "1,1,";
			} else {
				amount = amounts.get(vehicle).intValue();
			}
			out.print("\t { ");
			out.print(amount);
			out.print(", ");
			Integer existing = existingpositions.get(cc);
			if (existing == null) {
				out.print(offset);
				existingpositions.put(cc, new Integer(offset));
				offset += amount * 2;
				totalamount += amount * 2;
				carcolsoutput.add(cc);
			} else {
				out.print(existing.intValue());
			}
			out.println(" },");
		}
		out.println("};");
		out.println();
		out.println("char carcols[" + totalamount + "] = {");
		for (String s : carcolsoutput) {
			out.print("\t");
			out.println(s);
		}
		out.println("};");
	}
}

