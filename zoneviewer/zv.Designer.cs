namespace zv
{
	partial class zv
	{
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.IContainer components = null;

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		/// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
		protected override void Dispose(bool disposing)
		{
			if (disposing && (components != null))
			{
				components.Dispose();
			}
			base.Dispose(disposing);
		}

		#region Windows Form Designer generated code

		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.txtentry = new System.Windows.Forms.TextBox();
			this.img = new System.Windows.Forms.PictureBox();
			this.label1 = new System.Windows.Forms.Label();
			this.list = new System.Windows.Forms.ListBox();
			((System.ComponentModel.ISupportInitialize)(this.img)).BeginInit();
			this.SuspendLayout();
			// 
			// txtentry
			// 
			this.txtentry.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.txtentry.Font = new System.Drawing.Font("Courier New", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
			this.txtentry.Location = new System.Drawing.Point(618, 28);
			this.txtentry.Multiline = true;
			this.txtentry.Name = "txtentry";
			this.txtentry.ScrollBars = System.Windows.Forms.ScrollBars.Both;
			this.txtentry.Size = new System.Drawing.Size(254, 257);
			this.txtentry.TabIndex = 0;
			this.txtentry.TextChanged += new System.EventHandler(this.txtentry_TextChanged);
			// 
			// img
			// 
			this.img.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
				    | System.Windows.Forms.AnchorStyles.Left)
				    | System.Windows.Forms.AnchorStyles.Right)));
			this.img.Location = new System.Drawing.Point(12, 12);
			this.img.Name = "img";
			this.img.Size = new System.Drawing.Size(600, 600);
			this.img.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
			this.img.TabIndex = 1;
			this.img.TabStop = false;
			this.img.Paint += new System.Windows.Forms.PaintEventHandler(this.img_Paint);
			// 
			// label1
			// 
			this.label1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.label1.AutoSize = true;
			this.label1.Location = new System.Drawing.Point(618, 12);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(247, 13);
			this.label1.TabIndex = 2;
			this.label1.Text = "Enter entries here: (X1, Y1, Z1, X2, Y2, Z2, NAME)";
			// 
			// list
			// 
			this.list.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
				    | System.Windows.Forms.AnchorStyles.Right)));
			this.list.Font = new System.Drawing.Font("Courier New", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
			this.list.FormattingEnabled = true;
			this.list.ItemHeight = 16;
			this.list.Location = new System.Drawing.Point(618, 291);
			this.list.Name = "list";
			this.list.Size = new System.Drawing.Size(254, 308);
			this.list.TabIndex = 3;
			this.list.SelectedIndexChanged += new System.EventHandler(this.list_SelectedIndexChanged);
			// 
			// zv
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(884, 629);
			this.Controls.Add(this.list);
			this.Controls.Add(this.label1);
			this.Controls.Add(this.txtentry);
			this.Controls.Add(this.img);
			this.MinimumSize = new System.Drawing.Size(892, 656);
			this.Name = "zv";
			this.Text = "Form1";
			this.Resize += new System.EventHandler(this.zv_Resize);
			((System.ComponentModel.ISupportInitialize)(this.img)).EndInit();
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.TextBox txtentry;
		private System.Windows.Forms.PictureBox img;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.ListBox list;
	}
}

