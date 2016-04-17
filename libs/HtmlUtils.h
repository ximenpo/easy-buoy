#if	!defined(HTMLUTILS_H__089BB269_EC67_495d_885A_CAB04219E370__INCLUDED_)
#define	HTMLUTILS_H__089BB269_EC67_495d_885A_CAB04219E370__INCLUDED_

/* DrawHTML()
 * Drop-in replacement for DrawText() supporting a tiny subset of HTML.
 */

#if defined __cplusplus
extern "C"
#endif
int __stdcall DrawHTML(
                       HDC		hdc,		// handle of device context
                       LPCTSTR	lpString,	// address of string to draw
                       int		nCount,		// string length, in characters
                       LPRECT	lpRect,		// address of structure with formatting dimensions
                       UINT		uFormat,	// text-drawing flags
					   UINT		uLineSpace = 0		// 行间空白高度
                      );

int StripHTML(const char *html, char *plain,  size_t dwPlainSize);

#endif	//HTMLUTILS_H__089BB269_EC67_495d_885A_CAB04219E370__INCLUDED_