/*
 * Copyright (C) 2013 SheenFigure
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
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Collections.Generic;
using SheenFigure.Graphics;

namespace SheenFigure
{
    class CachedPage
    {
        private int m_StartIndex;
        private WriteableBitmap m_Bitmap;
        private Image m_BitmapView;

        public CachedPage(int startIndex, WriteableBitmap bitmap, float scale)
        {
            m_StartIndex = startIndex;
            m_Bitmap = bitmap;
            m_BitmapView = new Image()
            {
                HorizontalAlignment = System.Windows.HorizontalAlignment.Left,
                VerticalAlignment = System.Windows.VerticalAlignment.Top,
                Width = m_Bitmap.PixelWidth / scale,
                Height = m_Bitmap.PixelHeight / scale,
                Source = m_Bitmap,
                Stretch = Stretch.Fill
            };
        }

        public int StartIndex
        {
            get { return m_StartIndex; }
        }

        public WriteableBitmap Bitmap
        {
            get { return m_Bitmap; }
            set { m_Bitmap = value; m_BitmapView.Source = m_Bitmap; }
        }

        public Image BitmapView
        {
            get { return m_BitmapView; }
            set { m_BitmapView = value; }
        }
    }

    [TemplatePart(Name = "LayoutRoot", Type = typeof(Grid))]
    public class Label : Control
    {
        public static DependencyProperty TextProperty = DependencyProperty.Register(
            "Text", typeof(string), typeof(Label), null);

        public static DependencyProperty TextColorProperty = DependencyProperty.Register(
            "TextColor", typeof(Color), typeof(Label), null);

        public static DependencyProperty TextAlignmentProperty = DependencyProperty.Register(
            "TextAlignment", typeof(SheenFigure.Graphics.TextAlignment), typeof(Label), null);

        public static DependencyProperty WritingDirectionProperty = DependencyProperty.Register(
            "WritingDirection", typeof(SheenFigure.Graphics.WritingDirection), typeof(Label), null);

        private static readonly Color DEFAULT_COLOR = Color.FromArgb(255, 0, 0, 0);

        private static readonly int PADDING = 3;

        private Grid m_Grid;
        private Text m_Text;

        private List<CachedPage> m_CachedPages;

        private int m_MeasuredHeight;
        private int m_LinesInPage;

        private Boolean m_Drawn;

        public Label()
        {
            DefaultStyleKey = typeof(Label);

            m_Text = new Text();
            m_CachedPages = new List<CachedPage>();

            object textColor = base.GetValue(TextColorProperty);
            if (textColor == null)
                this.TextColor = DEFAULT_COLOR;
        }

        protected override Size MeasureOverride(Size availableSize)
        {
            m_Drawn = false;
            m_MeasuredHeight = 0;

            if (double.IsNaN(availableSize.Width))
            {
                if (m_Grid != null)
                {
                    TextBlock error = new TextBlock()
                    {
                        Text = "Width is not provided.",
                        FontSize = 30,
                        Foreground = new SolidColorBrush(Color.FromArgb(255, 255, 0, 0))
                    };

                    m_Grid.Children.Clear();
                    m_Grid.Children.Add(error);
                }
            }
            else
            {
                RefreshDrawing(availableSize.Width);
                availableSize.Height = m_MeasuredHeight;
            }

            return base.MeasureOverride(availableSize);
        }

        public override void OnApplyTemplate()
        {
            base.OnApplyTemplate();

            if (m_Grid == null)
            {
                m_Grid = this.GetTemplateChild("MainGrid") as Grid;

                m_Drawn = false;
                m_MeasuredHeight = 0;
                this.UpdateLayout();
            }
        }

        private int GetMaxAllowedPixelsInAPage()
        {
            int maxPixels = (int)(Application.Current.Host.Content.ActualWidth * Application.Current.Host.Content.ActualHeight);

            return maxPixels;
        }

        private void ClearCachedPages()
        {
            m_Grid.Children.Clear();
            m_CachedPages.Clear();

            GC.Collect();
        }

        private static void RenderGlyph(int[] pixels, int w, int h, float x, float y, object resObj)
        {
            WriteableBitmap source = (WriteableBitmap)resObj;

            int posX, limX, posY, orgX, difA;
            int sp, sa, sr, sg, sb;
            int dp, da, dr, dg, db;

            int sx = (int)x;
            int sy = (int)y;

            if (sy < 0)
                posY = -sy;
            else
                posY = 0;

            for (; posY < h; posY++)
            {
                if ((sy + posY) >= source.PixelHeight)
                    break;

                if (sx < 0)
                    posX = -sx;
                else
                    posX = 0;

                orgX = ((sy + posY) * source.PixelWidth) + Math.Max(0, sx);
                limX = ((sy + posY + 1) * source.PixelWidth) - 1;

                for (; posX < w && orgX < limX; posX++, orgX++)
                {
                    sp = pixels[(posY * w) + posX];
                    sa = (sp >> 24) & 0xFF;
                    sr = (sp >> 16) & 0xFF;
                    sg = (sp >> 8) & 0xFF;
                    sb = sp & 0xFF;

                    dp = source.Pixels[orgX];
                    da = (dp >> 24) & 0xFF;
                    dr = (dp >> 16) & 0xFF;
                    dg = (dp >> 8) & 0xFF;
                    db = dp & 0xFF;

                    difA = (0xFF - sa);
                    da = ((sa * sa) + (difA * da)) >> 8;
                    dr = ((sa * sr) + (difA * dr)) >> 8;
                    dg = ((sa * sg) + (difA * dg)) >> 8;
                    db = ((sa * sb) + (difA * db)) >> 8;

                    dp = (da << 24) | (dr << 16) | (dg << 8) | db;

                    source.Pixels.SetValue(dp, orgX);
                }
            }
        }

        private int GenerateCachedPages(double width)
        {
            ClearCachedPages();

            float lineHeight = m_Text.Font.Leading;

            float posX = PADDING;
            float posY = lineHeight / 2.0f;

            float scale = Windows.Graphics.Display.DisplayProperties.LogicalDpi / 96.0f;

            int imageWidth = (int)(width - Padding.Right - Padding.Left);
            int imageHeight = GetMaxAllowedPixelsInAPage() / imageWidth;

            float frameWidth = imageWidth - (PADDING * 2);
            m_LinesInPage = (int)Math.Floor((imageHeight - (lineHeight)) / (lineHeight));
            float pageHeight = lineHeight * m_LinesInPage;

            int countLines = 0;
            int totalLines = 0;

            int index = 0;
            int prevIndex = 0;

            for (int i = 0; index > -1; i++)
            {
                prevIndex = index;
                countLines = m_LinesInPage;

                index = m_Text.GetNextLineCharIndex(frameWidth, index, countLines);
                if (index < 0)
                    countLines = -index - 1;

                imageHeight = (int)(lineHeight * (countLines + 1));

                WriteableBitmap bmp = new WriteableBitmap((int)(imageWidth * scale), (int)(imageHeight * scale));
                m_Text.ShowString(RenderGlyph, frameWidth, posX, posY, prevIndex, countLines, bmp);

                CachedPage page = new CachedPage(prevIndex, bmp, scale);
                m_CachedPages.Add(page);

                Image bmpView = page.BitmapView;
                bmpView.Margin = new Thickness(Padding.Left, Padding.Top + ((i * pageHeight) - posY), Padding.Right, Padding.Bottom);

                m_Grid.Children.Add(bmpView);

                totalLines += countLines;
            }

            m_Drawn = true;
            m_MeasuredHeight = (int)Math.Round((totalLines * lineHeight) + Padding.Top + Padding.Bottom);

            return totalLines;
        }

        private void UpdateCachedPages()
        {
            Canvas canvas = new Canvas();

            float posX = PADDING;
            float posY = m_Text.Font.Leading / 2.0f;

            for (int i = 0; i < m_CachedPages.Count; i++)
            {
                CachedPage prevPage = m_CachedPages[i];

                int imageWidth = prevPage.Bitmap.PixelWidth;
                int imageHeight = prevPage.Bitmap.PixelHeight;

                int frameWidth = imageWidth - (PADDING * 2);

                prevPage.Bitmap = null;

                WriteableBitmap bmp = new WriteableBitmap(imageWidth, imageHeight);
                m_Text.ShowString(RenderGlyph, frameWidth, posX, posY, prevPage.StartIndex, m_LinesInPage, bmp);

                prevPage.Bitmap = bmp;
            }

            m_Drawn = true;
        }

        private void RefreshDrawing(double width)
        {
            if (m_Grid == null)
                return;

            Font font = m_Text.Font;
            String text = m_Text.String;

            width = (int)(width - Padding.Right - Padding.Left);
            if (width > 0 && font != null && text != null && text.Length > 0)
            {
                if (!m_Drawn)
                {
                    if (m_MeasuredHeight == 0)
                        GenerateCachedPages(width);
                    else
                        UpdateCachedPages();
                }
            }
            else
            {
                ClearCachedPages();
                m_Drawn = true;
            }
        }

        public string Text
        {
            get
            {
                var obj = base.GetValue(TextProperty);
                if (obj == null)
                    return string.Empty;

                return (string)obj;
            }
            set
            {
                if (value == null)
                    value = "";

                base.SetValue(TextProperty, value);
                m_Text.String = value;

                m_Drawn = false;
                m_MeasuredHeight = 0;
                this.UpdateLayout();
            }
        }

        public Font Font
        {
            get
            {
                return m_Text.Font;
            }
            set
            {
                if (value != m_Text.Font)
                {
                    m_Text.Font = value;

                    m_Drawn = false;
                    m_MeasuredHeight = 0;
                    this.UpdateLayout();
                }
            }
        }

        public Color TextColor
        {
            get
            {
                var obj = base.GetValue(TextColorProperty);
                if (obj == null)
                    return DEFAULT_COLOR;

                return (Color)obj;
            }
            set
            {
                if (value == null)
                    value = DEFAULT_COLOR;

                Windows.UI.Color c = m_Text.Color;
                if (value.R != c.R || value.G != c.G || value.B != c.B)
                {
                    base.SetValue(TextColorProperty, value);
                    m_Text.Color = Windows.UI.Color.FromArgb(value.A, value.R, value.G, value.B);

                    m_Drawn = false;
                    this.UpdateLayout();
                }
            }
        }

        public SheenFigure.Graphics.TextAlignment TextAlignment
        {
            get
            {
                return m_Text.Alignment;
            }
            set
            {
                if (value != m_Text.Alignment)
                {
                    base.SetValue(TextAlignmentProperty, value);
                    m_Text.Alignment = value;

                    m_Drawn = false;
                    this.UpdateLayout();
                }
            }
        }

        public SheenFigure.Graphics.WritingDirection WritingDirection
        {
            get
            {
                return m_Text.WritingDirection;
            }
            set
            {
                if (value != m_Text.WritingDirection)
                {
                    base.SetValue(WritingDirectionProperty, value);
                    m_Text.WritingDirection = value;

                    m_Drawn = false;
                    m_MeasuredHeight = 0;
                    this.UpdateLayout();
                }
            }
        }
    }
}
