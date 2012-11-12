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

        public CachedPage(int startIndex, WriteableBitmap bitmap)
        {
            m_StartIndex = startIndex;
            m_Bitmap = bitmap;
        }

        public int StartIndex
        {
            get { return m_StartIndex; }
        }

        public WriteableBitmap Bitmap
        {
            get { return m_Bitmap; }
            set { m_Bitmap = value; }
        }
    }

    [TemplatePart(Name = "LayoutRoot", Type = typeof(Grid))]
    public class Label : Control
    {
        public static DependencyProperty TextProperty = DependencyProperty.Register(
            "Text", typeof(string), typeof(Label), null);

        public static DependencyProperty TextColorProperty = DependencyProperty.Register(
            "TextColor", typeof(Color), typeof(Label), null);

        public static DependencyProperty TextAlignProperty = DependencyProperty.Register(
            "TextAlign", typeof(SheenFigure.TextAlignment), typeof(Label), null);

        private static readonly int MIN_WIDTH = 100;
        private static readonly int MIN_HEIGHT = 50;
        private static readonly Color DEFAULT_COLOR = Color.FromArgb(255, 255, 255, 255);

        private static readonly int MAX_ALLOWED_PIXELS_IN_A_PAGE = 480 * 800;
        private static readonly int PADDING = 3;

        private Grid m_Grid;

        private Font m_Font;
        private Text m_Text;

        private List<CachedPage> m_CachedPages;

        private int m_Width;
        private int m_MeasuredHeight;
        private int m_PageHeight;
        private int m_LinesInPage;

        public Label()
        {
            DefaultStyleKey = typeof(Label);

            this.MinWidth = MIN_WIDTH;
            this.MinHeight = MIN_HEIGHT;

            m_Text = new Text();
            m_CachedPages = new List<CachedPage>();

            object textColor = base.GetValue(TextColorProperty);
            if (textColor == null || textColor.Equals(Color.FromArgb(0, 0, 0, 0)))
                this.TextColor = DEFAULT_COLOR;
        }

        private int CalculatePageHeight(int width)
        {
            return MAX_ALLOWED_PIXELS_IN_A_PAGE / width;
        }

        private void ClearCachedPages()
        {
            for (int i = m_CachedPages.Count - 1; i >= 0; i--)
            {
                m_CachedPages[i].Bitmap = null;
                m_CachedPages.RemoveAt(i);
            }

            GC.Collect();
        }

        private static void RenderGlyph(int[] pixels, int w, int h, float x, float y, object resObj)
        {
            WriteableBitmap source = (WriteableBitmap)resObj;

            int posX, posY, orgX, difA;
            int sp, sa, sr, sg, sb;
            int dp, da, dr, dg, db;

            int sx = (int)x;
            int sy = (int)y;

            for (posY = 0; posY < h; posY++)
            {
                orgX = ((sy + posY) * source.PixelWidth) + sx;

                for (posX = 0; posX < w; posX++, orgX++)
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

        private int CreateCachedPages(int width)
        {
            ClearCachedPages();

            float lineHeight = this.Font.GetLeading();
            m_PageHeight = CalculatePageHeight(width);
            m_LinesInPage = (int)Math.Floor((m_PageHeight - lineHeight) / lineHeight);

            int totalLines = 0;

            int index = 0;
            while (index > -1)
            {
                int prevIndex = index;
                index = m_Text.GetCharIndexAfterLines(index, m_LinesInPage);

                int measuredLines;
                int pageHeight;

                if (index < 0)
                {
                    measuredLines = -index;
                    pageHeight = (int)(lineHeight * (measuredLines + 1));
                }
                else
                {
                    measuredLines = m_LinesInPage;
                    pageHeight = m_PageHeight;
                }

                WriteableBitmap bmp = new WriteableBitmap(width, pageHeight);
                m_Text.RenderText(RenderGlyph, prevIndex, m_LinesInPage, 0, (int)lineHeight, bmp);

                CachedPage page = new CachedPage(prevIndex, bmp);
                m_CachedPages.Add(page);

                totalLines += measuredLines;
            }

            return totalLines;
        }

        private void RefreshDrawing(bool measureHeight)
        {
            if (this.Font == null || this.Text == string.Empty || m_Grid == null)
                return;

            m_Grid.Children.Clear();
            if (double.IsNaN(this.Width))
            {
                TextBlock error = new TextBlock()
                {
                    Text = "Width is not provided.",
                    FontSize = 30,
                    Foreground = new SolidColorBrush(Color.FromArgb(255, 255, 0, 0))
                };
                m_Grid.Children.Add(error);

                return;
            }

            if (measureHeight)
            {
                int width = (int)this.Width - (PADDING * 2);
                if (width >= MIN_WIDTH && width != m_Width)
                {
                    m_Text.SetViewArea(width);

                    int lines = CreateCachedPages(width);
                    m_MeasuredHeight = (int)Math.Round((lines * this.Font.GetLeading()));

                    if (m_MeasuredHeight < MIN_HEIGHT)
                        m_MeasuredHeight = MIN_HEIGHT;

                    m_Width = width;
                }
            }

            float lineHeight = this.Font.GetLeading();
            float correctHeight = lineHeight * m_LinesInPage;

            for (int i = 0; i < m_CachedPages.Count; i++)
            {
                if (m_CachedPages[i].Bitmap == null)
                    continue;

                WriteableBitmap bmp = m_CachedPages[i].Bitmap;
                Thickness margins = new Thickness(PADDING, (i * correctHeight) - lineHeight, 0, 0);
                Image image = new Image()
                {
                    Margin = margins,
                    HorizontalAlignment = System.Windows.HorizontalAlignment.Left,
                    VerticalAlignment = System.Windows.VerticalAlignment.Top,
                    Width = bmp.PixelWidth,
                    Height = bmp.PixelHeight,
                    Source = bmp
                };

                m_Grid.Children.Add(image);
            }
        }

        protected override Size MeasureOverride(Size availableSize)
        {
            if (m_MeasuredHeight == 0)
                return new Size(MIN_WIDTH, MIN_HEIGHT);

            availableSize.Height = m_MeasuredHeight;
            availableSize.Width = base.MeasureOverride(availableSize).Width;

            return availableSize;
        }

        public override void OnApplyTemplate()
        {
            base.OnApplyTemplate();

            if (m_Grid == null)
            {
                m_Grid = this.GetTemplateChild("MainGrid") as Grid;

                this.UpdateLayout();
                this.InvalidateMeasure();
            }

            RefreshDrawing(true);
        }

        new public double Width
        {
            get
            {
                return base.Width;
            }
            set
            {
                if (base.Width != value && value > MIN_WIDTH)
                {
                    base.Width = value;

                    RefreshDrawing(true);
                }
            }
        }

        public Font Font
        {
            get
            {
                return m_Font;
            }
            set
            {
                if (m_Font != value)
                {
                    m_Font = value;

                    m_Text.ChangeFont(m_Font);
                    m_Text.ChangeText(this.Text);

                    RefreshDrawing(true);
                }
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
                m_Text.ChangeText(value);

                RefreshDrawing(true);
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

                base.SetValue(TextColorProperty, value);

                Color c = (Color)value;
                m_Text.SetColor(Windows.UI.Color.FromArgb(c.A, c.R, c.G, c.B));

                RefreshDrawing(false);
            }
        }

        public SheenFigure.TextAlignment TextAlign
        {
            get
            {
                var obj = base.GetValue(TextAlignProperty);
                if (obj == null)
                    return SheenFigure.TextAlignment.Right;

                return (SheenFigure.TextAlignment)obj;
            }
            set
            {
                base.SetValue(TextAlignProperty, value);
                m_Text.SetTextAlignment((SheenFigure.TextAlignment)value);

                RefreshDrawing(false);
            }
        }
    }
}
