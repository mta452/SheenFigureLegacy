/*
 * Copyright (C) 2012 SheenFigure
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

using System;
using System.IO;
using System.IO.IsolatedStorage;
using System.Reflection;
using System.Windows;
using System.Windows.Media;
using System.Windows.Controls;
using Microsoft.Phone.Controls;

namespace SheenFigure_WindowsPhone
{
    public partial class MainPage : PhoneApplicationPage
    {
        // Constructor
        public MainPage()
        {
            InitializeComponent();

            string fontPath = CopyFile("NafeesWeb.ttf");
            if (File.Exists(fontPath))
                SheenFigureLabel.Font = new SheenFigure.Graphics.Font(fontPath, 27.0f);
        }

        public static void CopyFile(Stream input, Stream output)
        {
            long length = input.Length;
            byte[] buffer = new byte[1024];
            int read = 0;
            int segment;
            while (read < length)
            {
                read += segment = input.Read(buffer, 0, buffer.Length);
                output.Write(buffer, 0, segment);
            }
        }

        public static string CopyFile(string filename)
        {
            IsolatedStorageFile isoStore = IsolatedStorageFile.GetUserStoreForApplication();

            Stream input = null;
            Stream output = null;

            string absoulutePath = Windows.Storage.ApplicationData.Current.LocalFolder.Path + '\\' + filename;

            if (!File.Exists(absoulutePath))
            {
                input = Application.GetResourceStream(new Uri(filename, UriKind.Relative)).Stream;
                output = new IsolatedStorageFileStream(filename, FileMode.CreateNew, isoStore);

                CopyFile(input, output);

                input.Close();
                input = null;

                output.Flush();
                output.Close();
                output = null;
            }

            return absoulutePath;
        }
    }
}