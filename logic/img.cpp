#include	"stdafx.h"

#include	<algorithm>
#include	"simple/timestamp.h"

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
static	size_t		g_frame_count	= 0;
static	double*		g_frame_elapse	= NULL;

bool	img_fetch_size(SIZE* sz){
	if(NULL == g_img || NULL == sz){
		return	false;
	}

	sz->cx	= (LONG)g_img->GetWidth();
	sz->cy	= (LONG)g_img->GetHeight();
	return	true;
}

bool	img_is_animation(){
	return	g_frame_count > 1;
}

void	img_destroy(){
	delete	g_img;
	g_img	= NULL;

	delete	g_frame_elapse;
	g_frame_elapse	= NULL;

	g_frame_count	= 0;
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
		g_frame_count	= g_img->GetFrameCount(&pDimensionIDs[0]);
		delete []pDimensionIDs;
	}

	// frame elapse
	if(g_frame_count > 1){
		int size			= g_img->GetPropertyItemSize(PropertyTagFrameDelay);
		PropertyItem* pItem	= (PropertyItem*)new char[size];
		g_img->GetPropertyItem(PropertyTagFrameDelay, size, pItem);
		g_frame_elapse	= new double[g_frame_count];
		double	elapse	= 0;
		for(size_t i = 0; i < g_frame_count; ++i){
			elapse	+= ((long*)pItem->value)[i] * 10 / 1000.0;
			g_frame_elapse[i]	= elapse;
		}
		delete[] (char*)pItem;
	}
	
	return	(g_frame_count >= 1);
}

bool	img_render(HDC hdc){
	if(NULL == g_img){
		return	false;
	}

	// select current frame
	if(g_frame_count > 1){
		static	timestamp	g_timestamp;
		double*	pframe	= std::lower_bound(g_frame_elapse, g_frame_elapse + g_frame_count, g_timestamp.now());
		g_img->SelectActiveFrame(&FrameDimensionTime, UINT(pframe - g_frame_elapse));
		if(g_timestamp.now() >= g_frame_elapse[g_frame_count - 1]){
			g_timestamp.reset();
		}
	}

	Graphics graphics(hdc);  
    graphics.DrawImage(g_img, 0, 0, g_img->GetWidth(), g_img->GetHeight());

	return	true;
}
