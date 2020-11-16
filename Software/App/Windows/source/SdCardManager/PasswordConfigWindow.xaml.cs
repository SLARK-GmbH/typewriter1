using System.Windows;

namespace SdCardManager
{
    /// <summary>
    /// Interaction logic for PasswordConfigWindow.xaml
    /// </summary>
    public partial class PasswordConfigWindow : Window
    {
        public PasswordConfigWindow()
        {
            InitializeComponent();
        }

        private void btnCancel_Click(object sender, RoutedEventArgs e)
        {
            Close();
        }

        private void btnOk_Click(object sender, RoutedEventArgs e)
        {
            if (string.IsNullOrEmpty(txtValidChars.Text))
            {
                MessageBox.Show("Please fill Valid Characters");
            }
            this.DialogResult = true;
        }
    }
}
