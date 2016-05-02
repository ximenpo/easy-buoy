#include	"stdafx.h"

#include	<GdiPlus.h>
#pragma		comment(lib, "Gdiplus.lib")
using namespace Gdiplus;

//////////////////////////////////////////////////////////////////
class	GdiplusInitializer{
public:
	GdiplusInitializer(){
		GdiplusStartupInput gdiplusStartupInput;
		GdiplusStartup(&gdiplusToken_, &gdiplusStartupInput, NULL);
	}
	~GdiplusInitializer(){
		GdiplusShutdown(gdiplusToken_);
	}

private:
	ULONG_PTR		gdiplusToken_;
};
static	GdiplusInitializer	s_gdiplus_initializer;
//////////////////////////////////////////////////////////////////

#include	"simple/string.h"

static	Image*		g_img	= NULL;
static	size_t		g_total_frames	= 0;
static	long*		g_frame_elapse	= NULL;
static	double		g_timestamp		= 0.0;

bool	img_fetch_size(SIZE* sz){
	if(NULL == g_img || NULL == sz){
		return	false;
	}

	sz->cx	= (LONG)g_img->GetWidth();
	sz->cy	= (LONG)g_img->GetHeight();
	return	true;
}

void	img_destroy(){
	delete	g_img;
	g_img	= NULL;

	delete	g_frame_elapse;
	g_frame_elapse	= NULL;

	g_total_frames	= 0;
}

bool	img_load(const char* file){
	if(NULL != g_img){
		img_destroy();
	}

	wchar_t	fn[MAX_PATH]	= {};
	string_utf8_to_wchar(file, strlen(file), fn, sizeof(fn));

	g_img	= new Image(fn);

	// fetch frames' count
	{
		UINT dim_count	= g_img->GetFrameDimensionsCount();
		if(dim_count <= 0){
			img_destroy();
			return	false;
		}
		
		GUID *pDimensionIDs	= (GUID*)new GUID[dim_count];
		g_img->GetFrameDimensionsList(pDimensionIDs, dim_count);
		g_total_frames	= g_img->GetFrameCount(&pDimensionIDs[0]);
		delete []pDimensionIDs;
	}

	// frame elapse
	if(g_total_frames > 1){
		int size			= g_img->GetPropertyItemSize(PropertyTagFrameDelay);
		PropertyItem* pItem	= (PropertyItem*)new char[size];
		g_img->GetPropertyItem(PropertyTagFrameDelay, size, pItem);
		g_frame_elapse	= new long[g_total_frames];
		long	elapse	= 0;
		for(size_t i = 0; i < g_total_frames; ++i){
			elapse	+= ((long*)pItem->value)[i] * 10;
			g_frame_elapse[i]	= elapse;
		}
		delete[] (char*)pItem;
	}
	
	return	(g_total_frames >= 1);
}

bool	img_render(HDC hdc, double timestamp){
	if(NULL == g_img){
		return	false;
	}

	// TODO: select current frame
	if(g_total_frames > 1 && NULL != g_frame_elapse){
		g_img->SelectActiveFrame(&FrameDimensionTime, 0);
	}

	Graphics graphics(hdc);  
    graphics.DrawImage(g_img, 0, 0, g_img->GetWidth(), g_img->GetHeight());

	return	true;
}
