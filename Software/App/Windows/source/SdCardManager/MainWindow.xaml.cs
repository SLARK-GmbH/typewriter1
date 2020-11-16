using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Media;
using System.Runtime.Serialization.Json;
using System.Text;
using System.Threading;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace SdCardManager
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private List<FileEntry> files = new List<FileEntry>();
        private SdCardManagerConfig config = new SdCardManagerConfig();
        private bool isLoaded = false;
        public static bool driveIsOn = true;
        public static bool g_close = false;
        private Thread threadCheckDrive = null;
        private System.Windows.Threading.DispatcherTimer timerCheckDriver;
        private int stopTimer = 4;
        public MainWindow()
        {
            InitializeComponent();

            gridFiles.Columns[0].Width = new DataGridLength(1, DataGridLengthUnitType.Auto);
            gridFiles.Columns[4].Width = new DataGridLength(1, DataGridLengthUnitType.Auto);

            foreach (DataGridColumn column in this.gridFiles.Columns)
            {
                column.CanUserSort = false;
                column.CanUserResize = false;
                column.CanUserReorder = false;
            }
            LoadConfig();
            threadCheckDrive = new Thread(DoWork);
            threadCheckDrive.Start(Path.GetPathRoot(this.AppFolder));

            timerCheckDriver = new System.Windows.Threading.DispatcherTimer();
            timerCheckDriver.Tick += dispatcherTimer_Tick;
            timerCheckDriver.Interval = new TimeSpan(0, 0, 1);
            timerCheckDriver.Start();
        }
        private void dispatcherTimer_Tick(object sender, EventArgs e)
        {
            if (g_close)
            {
                timerCheckDriver.Stop();
                return;
            }
            if (!driveIsOn)
            {
                mainGrid.IsEnabled = false;
                gridFiles.Visibility = Visibility.Collapsed;
                labelNoFiles.Visibility = Visibility.Visible;
                stopTimer--;
                if (stopTimer<=0)
                {
                    timerCheckDriver.Stop();
                    Close();
                }
                labelNoFiles.Text = string.Format("Drive has been removed. Auto-close in {0} seconds", stopTimer);
            }
        }

        public static void DoWork(object data)
        {
            string drive = data.ToString();
            while (!g_close && driveIsOn)
            {
                driveIsOn = Directory.Exists(drive);
                Thread.Sleep(2000);
            }
        }
        private string AppFolder
        {
            get
            {
                return System.IO.Path.GetDirectoryName(System.Diagnostics.Process.GetCurrentProcess().MainModule.FileName);
            }
        }
        private void LoadConfig()
        {
            string configFolder = System.IO.Path.Combine(AppFolder, "typewriter1/config");
            configFolder = System.IO.Path.Combine(configFolder, "app.config");
            if (File.Exists(configFolder))
            {
                string txt = File.ReadAllText(configFolder);
                try
                {

                    using (Stream stream = File.OpenRead(configFolder))
                    {
                        DataContractJsonSerializer serializer = new DataContractJsonSerializer(typeof(SdCardManagerConfig));
                        config = (SdCardManagerConfig)serializer.ReadObject(stream);
                    }
                    if (config.PasswordLength < 1)
                    {
                        throw new Exception("Invalid PasswordLength");
                    }
                    if (string.IsNullOrEmpty(config.ValidChars))
                    {
                        throw new Exception("ValidChars cannot be empty");
                    }
                    txtLength.Text = config.PasswordLength.ToString();
                    cbAdvanced.IsChecked = config.IsAdvanced;
                    cbShowPasswords.IsChecked = config.IsShowPassword;
                }
                catch (Exception exc)
                {
                    MessageBox.Show("Cannot load Appplication's oOnfig. " + exc.Message, "Warning", MessageBoxButton.OK, MessageBoxImage.Warning);
                }
            }
        }
        private void SaveConfig()
        {
            string configFolder = System.IO.Path.Combine(AppFolder, "typewriter1/config");
            try
            {
                if (!Directory.Exists(configFolder))
                {
                    Directory.CreateDirectory(configFolder);
                }
                string path = System.IO.Path.Combine(configFolder, "app.config");
                if (File.Exists(path))
                {
                    File.Delete(path);
                }
                DataContractJsonSerializer serializer = new DataContractJsonSerializer(typeof(SdCardManagerConfig));
                using (var stream = File.Create(path))
                {
                    serializer.WriteObject(stream, config);
                }
            }
            catch (Exception exc)
            {
                MessageBox.Show("Error create Config folder" + exc.Message, "Error", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        private void cbUserNameCheckChanged()
        {
            if (gridFiles == null)
            {
                return;
            }
            panelUserName.Visibility = cbUserName.IsChecked == true ? Visibility.Visible : Visibility.Collapsed;
            mainGrid.RowDefinitions[0].Height = new GridLength(cbUserName.IsChecked == true ? 120 : 90);
        }
        private void cbAdvancedChanged()
        {
            if (gridFiles == null)
            {
                return;
            }
            gridFiles.Columns[3].Visibility = cbAdvanced.IsChecked == true ? Visibility.Visible : Visibility.Collapsed;
            gridFiles.Columns[4].Visibility = gridFiles.Columns[3].Visibility;
            
            if (isLoaded)
            {
                config.IsAdvanced = cbAdvanced.IsChecked == true;
                SaveConfig();
            }
            //gridFiles.Columns[5].Visibility = cbShowPasswords.IsChecked == true ? Visibility.Visible : Visibility.Collapsed;
        }
        private void CheckBox_Checked(object sender, RoutedEventArgs e)
        {
            cbUserNameCheckChanged();
        }

        private void cbUserName_Unchecked(object sender, RoutedEventArgs e)
        {
            cbUserNameCheckChanged();
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            string fileName = ((Button)sender).Tag.ToString();
            FileEntry fe = files.First(r => r.FullPath == fileName);
            string _type = fe.FileType;
            string displayName = _type.Length == 4 ? string.Format("{0}-{1}", fe.DisplayName, fe.FileType) : fe.DisplayName;

            if (MessageBox.Show(string.Format("Are you sure you want to delete file {0}?", displayName), "Confirmation", MessageBoxButton.YesNo, MessageBoxImage.Question) != MessageBoxResult.Yes)
            {
                return;
            }
            if (MessageBox.Show(string.Format("Are you REALLY sure you want to delete file {0}?", displayName), "Last confirmation", MessageBoxButton.YesNo, MessageBoxImage.Question) != MessageBoxResult.Yes)
            {
                return;
            }
            try
            {
                File.Delete(fe.FullPath);
            }
            catch (Exception exc)
            {
                MessageBox.Show(exc.Message, "Error", MessageBoxButton.OK, MessageBoxImage.Error);
            }
            GetFiles();
        }

        private void cbAdvanced_Checked(object sender, RoutedEventArgs e)
        {
            cbAdvancedChanged();
        }

        private void cbAdvanced_Unchecked(object sender, RoutedEventArgs e)
        {
            cbAdvancedChanged();
        }

        private void gridFiles_CellEditEnding(object sender, DataGridCellEditEndingEventArgs e)
        {
            if (e.EditAction != DataGridEditAction.Commit)
            {
                return;
            }
            TextBox txt = null;
            if (e.EditingElement != null)
            {
                if (e.EditingElement is TextBox)
                {
                    txt = e.EditingElement as TextBox;
                }
            }
            if (txt == null)
            {
                return;
            }
            FileEntry fe = ((FileEntry)e.Row.Item);
            string fileName = fe.FullPath;
            string _type = fe.FileType;
            //btnCreatePassword.Content = e.Column.DisplayIndex.ToString() + " " + ((FileEntry)e.Row.Item).FullPath;
            switch (e.Column.DisplayIndex)
            {
                case 0:  // DisplayName

                    if (string.IsNullOrEmpty(txt.Text))
                    {
                        MessageBox.Show("Empty name is not allowed");
                    }
                    else
                    {
                        if (files.FirstOrDefault(r => string.Equals(r.DisplayName, txt.Text, StringComparison.OrdinalIgnoreCase) &&
                        string.Equals(r.FileType, fe.FileType, StringComparison.OrdinalIgnoreCase) &&
                        !string.Equals(r.FullPath, fileName, StringComparison.OrdinalIgnoreCase)
                        ) != null)
                        {
                            MessageBox.Show("Same entry already exists");
                        }
                        else
                        {
                            if (checkFileName(txt.Text))
                            {
                                if (
                                    string.Equals(_type, "user", StringComparison.OrdinalIgnoreCase) ||
                                    string.Equals(_type, "pass", StringComparison.OrdinalIgnoreCase) ||
                                    textIs4Chars(_type)
                                    )
                                {
                                    File.Delete(fe.FullPath);
                                    File.AppendAllText(Path.Combine(this.AppFolder, txt.Text + _type + ".txt"), GenerateFileTextContent(fe.Content, txt.Text, _type));
                                }
                                else
                                {
                                    SystemSounds.Beep.Play();
                                }
                            }
                        }
                    }
                    break;
                case 1:  // type
                    if (files.FirstOrDefault(r => string.Equals(r.DisplayName, fe.DisplayName, StringComparison.OrdinalIgnoreCase) &&
                                            string.Equals(r.FileType, txt.Text, StringComparison.OrdinalIgnoreCase) &&
                                            !string.Equals(r.FullPath, fileName, StringComparison.OrdinalIgnoreCase)
                                            ) != null)
                    {
                        MessageBox.Show("Same entry already exists");
                    }
                    else
                    {
                        if (string.Equals(txt.Text, "user", StringComparison.OrdinalIgnoreCase) ||
                            string.Equals(txt.Text, "pass", StringComparison.OrdinalIgnoreCase)
                            )
                        {
                            File.Delete(fe.FullPath);
                            File.AppendAllText(Path.Combine(this.AppFolder, fe.DisplayName + txt.Text + ".txt"), GenerateFileTextContent(fe.Content, fe.DisplayName, txt.Text));
                        }
                        else
                        {
                            if (!textIs4Chars(txt.Text))
                            {
                                MessageBox.Show("You can enter next types: 'user', 'pass' or 4 characters");
                            }
                            else if (checkFileName(fe.DisplayName + txt.Text))
                            {

                                File.Delete(fe.FullPath);
                                File.AppendAllText(Path.Combine(this.AppFolder, fe.DisplayName + txt.Text + ".txt"), GenerateFileTextContent(fe.Content, fe.DisplayName, txt.Text));
                            }
                        }
                    }
                    break;
                case 5: // content
                    if (txt.Text == fe.Content)
                    {
                        return;
                    }
                    if (MessageBox.Show(string.Format("Are you sure you want to change entry {0}-{1}?", fe.DisplayName, _type), "Confirmation", MessageBoxButton.YesNo, MessageBoxImage.Question) == MessageBoxResult.Yes)
                    {
                        if (MessageBox.Show(string.Format("Are you REALLY sure you want to change entry {0}-{1}?", fe.DisplayName, _type), "Confirmation", MessageBoxButton.YesNo, MessageBoxImage.Question) == MessageBoxResult.Yes)
                        {
                            if (
                                string.Equals(_type, "user", StringComparison.OrdinalIgnoreCase) ||
                                string.Equals(_type, "pass", StringComparison.OrdinalIgnoreCase) ||
                                textIs4Chars(_type)
                                )
                            {
                                File.Delete(fe.FullPath);
                                File.AppendAllText(fe.FullPath, GenerateFileTextContent(txt.Text, fe.DisplayName, _type));
                            }
                            else
                            {
                                SystemSounds.Beep.Play();
                            }

                        }
                    }
                    break;
            }
            GetFiles();

            FileEntry feSelect = files.FirstOrDefault(r => r.FullPath == fileName);
            if (feSelect != null)
            {
                gridFiles.SelectedItems.Clear();
                int i = files.IndexOf(feSelect);
                object item = gridFiles.Items[i];
                gridFiles.SelectedItem = item;
                gridFiles.ScrollIntoView(item);
                DataGridRow row = (DataGridRow)gridFiles.ItemContainerGenerator.ContainerFromIndex(i);
                row.MoveFocus(new TraversalRequest(FocusNavigationDirection.Previous));
                gridFiles.Focus();
            }

        }
        private bool textIs4Chars(string text)
        {
            if (string.IsNullOrEmpty(text))
            {
                return false;
            }
            return text.Length == 4;
        }
        private void gridFiles_BeginningEdit(object sender, DataGridBeginningEditEventArgs e)
        {
            string _type = ((FileEntry)e.Row.Item).FileType;
            if (!string.Equals(_type, "pass", StringComparison.OrdinalIgnoreCase) &&
                !string.Equals(_type, "user", StringComparison.OrdinalIgnoreCase) &&
                !textIs4Chars(_type)
                )
            {
                SystemSounds.Beep.Play();
                e.Cancel = true;
            }
        }


        private void gridFiles_Sorting(object sender, DataGridSortingEventArgs e)
        {
            //disable sorting
            //e.Handled = true;
        }

        private void btnCreatePassword_Click(object sender, RoutedEventArgs e)
        {
            if (string.IsNullOrEmpty(txtLength.Text))
            {
                MessageBox.Show("Please fill Password Length");
                return;
            }
            if (cbUserName.IsChecked == true && string.IsNullOrEmpty(txtUsername.Text))
            {
                MessageBox.Show("Please fill User Name");
                return;
            }
            if (string.IsNullOrEmpty(txtEntry.Text))
            {
                MessageBox.Show("Please fill Entry Name");
                return;
            }

            ushort passLen;
            if (!ushort.TryParse(txtLength.Text, out passLen))
            {
                MessageBox.Show("Cannot parse Password Length");
                return;
            }
            if (passLen == 0)
            {
                MessageBox.Show("Password Length should be more than zero");
                return;
            }
            if (cbUserName.IsChecked == true)
            {
                if (files.FirstOrDefault(r=>r.DisplayName == txtEntry.Text && r.FileType=="User") != null)
                {
                    MessageBox.Show("Same Entry with User already exists");
                    return;
                }
            }
            if (files.FirstOrDefault(r => r.DisplayName == txtEntry.Text && r.FileType == "Pass") != null)
            {
                MessageBox.Show("Same Entry with Pass already exists");
                return;
            }
            string chars = config.ValidChars + config.CharsMustContainAtLeastOne;
            Random rnd = new Random();
            int charsLen = chars.Length;
            StringBuilder sbPass = new StringBuilder();
            bool needCheckMustHave = !string.IsNullOrEmpty(config.CharsMustContainAtLeastOne);
            bool bMustHaveExists = !needCheckMustHave;
            int iCh;
            for (ushort i = 0; i < passLen; i++)
            {
                iCh = rnd.Next(0, charsLen - 1);
                char ch = chars[iCh];
                if (needCheckMustHave)
                {
                    if (config.CharsMustContainAtLeastOne.Contains(ch))
                    {
                        bMustHaveExists = true;
                    }
                }
                sbPass.Append(ch);
            }
            if (!bMustHaveExists)
            {
                int iPos = rnd.Next(0, passLen - 1);
                iCh = rnd.Next(0, config.CharsMustContainAtLeastOne.Length - 1);
                char ch = config.CharsMustContainAtLeastOne[iCh];
                sbPass.Remove(iPos, 1);
                sbPass.Insert(iPos, ch);
            }
            string fileName = txtEntry.Text + "user";
            if (!checkFileName(fileName))
            {
                return;
            }

            if (cbUserName.IsChecked == true)
            {
                File.AppendAllText(Path.Combine(this.AppFolder, txtEntry.Text + "user.txt"), GenerateFileTextContent(txtUsername.Text, txtEntry.Text, "user"));
            }

            File.AppendAllText(Path.Combine(this.AppFolder, txtEntry.Text + "pass.txt"), GenerateFileTextContent(sbPass.ToString(), txtEntry.Text, "pass"));

            GetFiles();
            config.PasswordLength = passLen;
            SaveConfig();

        }
        private bool checkFileName(string name)
        {
            char[] invalidFileChars = Path.GetInvalidFileNameChars();

            
            try
            {
                foreach (char ch in name.ToArray())
                {
                    if (invalidFileChars.Contains(ch))
                    {
                        throw new Exception($"Name contains invalid character {ch}");
                    }
                }
                string fileName = Path.Combine(this.AppFolder, name + ".txt");
                if (File.Exists(fileName))
                {
                    File.Delete(fileName);
                }
                File.AppendAllText(fileName, " ");
                File.Delete(fileName);
                return true;
            }
            catch (Exception exc)
            {
                MessageBox.Show(exc.Message, "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                return false;
            }
        }
        private string GenerateFileTextContent(string content, string entryName, string type)
        {
            StringBuilder sbFile = new StringBuilder();
            sbFile.AppendLine(content);
            string _type = type;
            if (string.Equals(_type, "user", StringComparison.OrdinalIgnoreCase))
            {
                _type = "User";
            }
            if (string.Equals(_type, "pass", StringComparison.OrdinalIgnoreCase))
            {
                _type = "Pass";
            }
            sbFile.AppendLine(entryName + " " + _type);
            return sbFile.ToString();
        }
        private void GetFiles()
        {
            if (!isLoaded)
            {
                return;
            }
            DirectoryInfo info = new DirectoryInfo(this.AppFolder);
            FileInfo[] _files = info.GetFiles("*.txt", SearchOption.TopDirectoryOnly).OrderBy(r => r.FullName).ToArray();
            files = new List<FileEntry>();
            int i = 0;
            foreach (FileInfo file in _files)
            {
                string[] lines;
                bool isTooBig = false;
                if (file.Length > 1000000)
                {
                    lines = new string[] { "Too big file", "FileName: " + Path.GetFileNameWithoutExtension(file.FullName) };
                    isTooBig = true;
                }
                else
                {
                    lines = File.ReadAllLines(file.FullName);
                }

                string _type = "Unknown";
                string displayName = "FileName: " + Path.GetFileNameWithoutExtension(file.FullName);
                if (lines.Length > 1)
                {
                    _type = lines[1];
                    if (_type.ToLower().EndsWith(" user"))
                    {
                        displayName = _type.Substring(0, _type.Length - 5);
                        _type = "User";
                    }
                    else if (_type.ToLower().EndsWith(" pass"))
                    {
                        displayName = _type.Substring(0, _type.Length - 5);
                        _type = "Pass";
                    }
                    else
                    {
                        int iPos = _type.LastIndexOf(' ');
                        if (iPos == _type.Length - 5)
                        {
                            displayName = _type.Substring(0, _type.Length - 5);
                            _type = _type.Substring(iPos + 1);
                        }
                    }
                }
                if (isTooBig)
                {
                    displayName = Path.GetFileNameWithoutExtension(file.FullName);
                    _type = "?";
                }
                string _content = lines[0];
                if (cbShowPasswords.IsChecked != true && string.Equals(_type, "pass", StringComparison.OrdinalIgnoreCase))
                {
                    _content = "*";
                }

                files.Add(new FileEntry()
                {
                    DisplayName = displayName,
                    Age = ((long)(DateTime.Now - file.LastWriteTime).TotalDays).ToString(),
                    Length = lines[0].Length,
                    Content = _content,
                    FileType = _type,
                    FullPath = file.FullName
                }
                );
                i++;
            }
            files = files.OrderBy(r => r.DisplayName).ToList();
            labelNoFiles.Visibility = files.Count > 0 ? Visibility.Collapsed : Visibility.Visible;
            gridFiles.Visibility = files.Count == 0 ? Visibility.Collapsed : Visibility.Visible;
            //gridFiles.Columns[5].MaxWidth = double.PositiveInfinity;
            gridFiles.ItemsSource = files;
            gridFiles.Columns[5].Width = new DataGridLength(1, DataGridLengthUnitType.SizeToCells);

        }

        private void btnRefresh_Click(object sender, RoutedEventArgs e)
        {
            GetFiles();
        }

        private void btnAdvanced_Click(object sender, RoutedEventArgs e)
        {
            PasswordConfigWindow w = new PasswordConfigWindow();
            w.txtMustContain.Text = config.CharsMustContainAtLeastOne;
            w.txtValidChars.Text = config.ValidChars;
            if (w.ShowDialog() == true)
            {
                config.CharsMustContainAtLeastOne = w.txtMustContain.Text;
                config.ValidChars = w.txtValidChars.Text;
                SaveConfig();
            }
        }

        private void cbShowPasswords_Checked(object sender, RoutedEventArgs e)
        {
            if (isLoaded)
            {
                config.IsShowPassword = cbShowPasswords.IsChecked == true;
                SaveConfig();
            }
            GetFiles();
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            if (isLoaded)
            {
                config.IsShowPassword = cbShowPasswords.IsChecked == true;
                SaveConfig();
            }
            isLoaded = true;
            GetFiles();
        }

        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            g_close = true;
        }
    }
}
