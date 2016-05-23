using System;
using System.Drawing;
using System.Collections;
using System.Windows.Forms;
using System.Data;

namespace GCQPPC
{
	/// <summary>
	/// Summary description for Form1.
	/// </summary>
	public class ChatForm : System.Windows.Forms.Form
	{
		private System.Windows.Forms.MenuItem menuItem1;
		private System.Windows.Forms.TextBox textBox1;
		private System.Windows.Forms.TextBox textBox2;
		private System.Windows.Forms.MenuItem menuItem2;
		private System.Windows.Forms.MenuItem menuItem3;
		private System.Windows.Forms.MenuItem menuItem4;
		private System.Windows.Forms.MenuItem menuItem5;
		private System.Windows.Forms.MenuItem menuItem6;
		private System.Windows.Forms.MenuItem menuItem7;
		private System.Windows.Forms.MainMenu mainMenu1;

		public ChatForm()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			//
			// TODO: Add any constructor code after InitializeComponent call
			//
		}
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			base.Dispose( disposing );
		}
		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.mainMenu1 = new System.Windows.Forms.MainMenu();
			this.menuItem1 = new System.Windows.Forms.MenuItem();
			this.textBox1 = new System.Windows.Forms.TextBox();
			this.textBox2 = new System.Windows.Forms.TextBox();
			this.menuItem2 = new System.Windows.Forms.MenuItem();
			this.menuItem3 = new System.Windows.Forms.MenuItem();
			this.menuItem4 = new System.Windows.Forms.MenuItem();
			this.menuItem5 = new System.Windows.Forms.MenuItem();
			this.menuItem6 = new System.Windows.Forms.MenuItem();
			this.menuItem7 = new System.Windows.Forms.MenuItem();
			// 
			// mainMenu1
			// 
			this.mainMenu1.MenuItems.Add(this.menuItem1);
			this.mainMenu1.MenuItems.Add(this.menuItem2);
			// 
			// menuItem1
			// 
			this.menuItem1.Text = "Send";
			// 
			// textBox1
			// 
			this.textBox1.Multiline = true;
			this.textBox1.ReadOnly = true;
			this.textBox1.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
			this.textBox1.Size = new System.Drawing.Size(240, 248);
			this.textBox1.Text = "GameCQ .. Type /? for a list of commands.";
			// 
			// textBox2
			// 
			this.textBox2.AcceptsReturn = true;
			this.textBox2.Location = new System.Drawing.Point(0, 248);
			this.textBox2.Size = new System.Drawing.Size(240, 22);
			this.textBox2.Text = "textBox2";
			// 
			// menuItem2
			// 
			this.menuItem2.MenuItems.Add(this.menuItem3);
			this.menuItem2.MenuItems.Add(this.menuItem4);
			this.menuItem2.MenuItems.Add(this.menuItem5);
			this.menuItem2.MenuItems.Add(this.menuItem6);
			this.menuItem2.MenuItems.Add(this.menuItem7);
			this.menuItem2.Text = "GameCQ";
			// 
			// menuItem3
			// 
			this.menuItem3.Text = "&Options";
			// 
			// menuItem4
			// 
			this.menuItem4.Text = "-";
			// 
			// menuItem5
			// 
			this.menuItem5.Text = "&Rooms";
			// 
			// menuItem6
			// 
			this.menuItem6.Text = "&Games";
			// 
			// menuItem7
			// 
			this.menuItem7.Text = "&Servers";
			// 
			// ChatForm
			// 
			this.BackColor = System.Drawing.Color.Gray;
			this.Controls.Add(this.textBox2);
			this.Controls.Add(this.textBox1);
			this.Menu = this.mainMenu1;
			this.Text = "GameCQ";

		}
		#endregion

		/// <summary>
		/// The main entry point for the application.
		/// </summary>

		static void Main() 
		{
			Application.Run(new ChatForm());
		}
	}
}
