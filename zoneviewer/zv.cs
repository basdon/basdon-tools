using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Globalization;
using System.Threading;

namespace zv
{
	public partial class zv : Form
	{
		[STAThread]
		static void Main()
		{
			Thread t = Thread.CurrentThread;
			CultureInfo c = (CultureInfo) t.CurrentCulture.Clone();
			c.NumberFormat.NumberDecimalSeparator = ".";
			t.CurrentCulture = c;

			Application.SetCompatibleTextRenderingDefault(true);
			Application.Run(new zv());
		}

		class Zone
		{
			public float x1, y1, z1, x2, y2, z2;
			public string name;
		}


		float scalex;
		float scaley;
		List<Zone> zones = new List<Zone>();

		public zv()
		{
			InitializeComponent();
			zv_Resize(null, null);
			try {
				img.ImageLocation = "map.png";
			} catch (Exception) {
				MessageBox.Show("could not find map.png in working dir");
			}
		}

		private void zv_Resize(object sender, EventArgs e)
		{
			scalex = (801f - 116f) / (2864.1099f + 2282.2373f);
			scaley = (745f - 74f) / (2441.9075f + 2579.7458f);
			scalex *= img.Width / 840f;
			scaley *= img.Height / 840f;
		}

		private void txtentry_TextChanged(object sender, EventArgs e)
		{
			list.Items.Clear();
			List<Zone> zones = new List<Zone>();
			string[] lines = txtentry.Text.Replace("\r\n", "\n").Split('\n');
			int idx = 0;
			foreach (string line in lines) {
				string[] parts = line.Split(new char[]{','}, 7);
				if (parts.Length != 7) {
					continue;
				}
				try {
					Zone z = new Zone();
					z.x1 = float.Parse(parts[0].Trim());
					z.y1 = -float.Parse(parts[1].Trim());
					z.z1 = float.Parse(parts[2].Trim());
					z.x2 = float.Parse(parts[3].Trim());
					z.y2 = -float.Parse(parts[4].Trim());
					z.z2 = float.Parse(parts[5].Trim());
					z.name = parts[6].Trim();
					zones.Add(z);
					list.Items.Add(++idx + " " + z.name);
				} catch (Exception) {
				}
			}
			this.zones = zones;
			img.Refresh();
		}

		private void img_Paint(object sender, PaintEventArgs e)
		{
			Graphics g = e.Graphics;
			Pen pen = new Pen(Color.Red);
			Brush brush = new SolidBrush(Color.FromArgb(180, Color.Maroon));
			Brush hibrush = new SolidBrush(Color.FromArgb(190, Color.Blue));
			Brush textcol = new SolidBrush(Color.White);
			Font font = new Font("Arial", 6);
			int selectedidx = list.SelectedIndex;
			int idx = 0;
			foreach (Zone z in zones) {
				float x = Math.Min(z.x1, z.x2) * scalex + img.Width / 2;
				float y = Math.Min(z.y1, z.y2) * scaley + img.Height / 2;
				float width = Math.Abs(z.x2 - z.x1) * scalex;
				float height = Math.Abs(z.y2 - z.y1) * scaley;
				Brush b = idx == selectedidx ? hibrush : brush;
				g.FillRectangle(b, x, y, width, height);
				g.DrawRectangle(pen, x, y, width, height);
				g.DrawString(z.name, font, textcol, x, y);
				idx++;
			}
		}

		private void list_SelectedIndexChanged(object sender, EventArgs e)
		{
			img.Refresh();
		}
	}
}