#if !defined(BITMAPHDC_H__089BB269_EC67_495d_885A_CAB04219E370__INCLUDED_)
#define BITMAPHDC_H__089BB269_EC67_495d_885A_CAB04219E370__INCLUDED_

//
//  ÄÚ´æBitmap»æÍ¼Àà
//
class BitmapHDC
{
public:
    BitmapHDC() : m_hDC(NULL), m_hOldBmp(NULL), m_hBitmap(NULL), m_nBmpWidth(0), m_nBmpHeight(0)
    {
    }

    virtual ~BitmapHDC()
    {
        UnInitialize();
    }

    void    Initialize(int nWidth, int nHeight, HDC hDC = NULL, COLORREF clrBackground = RGB(255,255,255))
    {
		UnInitialize();

		HDC dcScreen	= NULL;
        if (NULL == hDC)
        {
			hDC	= dcScreen	= ::GetDC(::GetDesktopWindow());
        }

        // make compatible device context and select bitmap into it
        m_hBitmap	= CreateCompatibleBitmap(hDC, nWidth, nHeight); 
		m_hDC		= CreateCompatibleDC(hDC);

		m_hOldBmp	= (HBITMAP)::SelectObject(m_hDC, m_hBitmap);
        ::SetMapMode(m_hDC, ::GetMapMode(hDC));

        // paint background in bitmap
		{
			RECT	rcRect	= {0, 0, nWidth, nHeight};
			::SetBkMode		(m_hDC, TRANSPARENT);
			::SetBkColor	(m_hDC, clrBackground);
			::ExtTextOut	(m_hDC, 0, 0, ETO_OPAQUE, &rcRect, NULL, 0, NULL);
		}

        m_nBmpWidth     = nWidth;
        m_nBmpHeight    = nHeight;

		if(NULL != dcScreen)
		{
			::ReleaseDC(::GetDesktopWindow(), dcScreen);
		}
    }

    void    UnInitialize()
	{
		m_nBmpWidth     = 0;
		m_nBmpHeight    = 0;

		if(NULL != m_hDC)
		{
			::DeleteDC(m_hDC);
			m_hDC	= NULL;
		}

        if(NULL != m_hBitmap)
        {
			::DeleteObject(m_hBitmap);
        }
    }

	void	SetUseBitmap(bool bSelect)
	{
		if(bSelect)
		{
			if(NULL != m_hBitmap)   ::SelectObject(m_hDC, m_hBitmap);
		}
		else
		{
			if(NULL != m_hOldBmp)   ::SelectObject(m_hDC, m_hOldBmp);
		}
	}

    HBITMAP     GetHBITMAP()
    {
        return  m_hBitmap;
    }

    int         GetBitmapWidth()
    {
        return  m_nBmpWidth;
    }

    int         GetBitmapHeight()
    {
        return  m_nBmpWidth;
    }

    HDC			GetHDC() 
    {
        return m_hDC;
    }
 
    operator	HDC() 
    {
        return m_hDC;
    }

protected:
	HDC			m_hDC;
    HBITMAP		m_hBitmap;
    HBITMAP		m_hOldBmp;
    int         m_nBmpWidth;
    int         m_nBmpHeight;
};


#endif  //BITMAPHDC_H__089BB269_EC67_495d_885A_CAB04219E370__INCLUDED_