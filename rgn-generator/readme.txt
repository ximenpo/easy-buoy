
The RGN Generator v1.01 (01/05/1999)
-----------------------------------------------------------------------------

The point of this program is to allow you 
to create non-rectangular dialog boxes.

The program itself is an example of 
what can happen if this is taken a bit too far.

Creating your transparency region
-----------------------------------------------------------------------------
1. Create a mask image for the dialog box. 
   One color will be the transparent part of the 
   dialog. A transparent gif is a good method 
   but many image formats are supported.
2. Run the RGNerator
3. Enter the filename of your mask.
   (as soon as a valid image file name is entered
   it will be automatically loaded)
   Browse for it if you cannot remember 
   the name or where you put it.
4. Enter the color that represents the transparent 
   portion of the dialog.
   If you do not know the exact RGB color press 
   the Pick button.
   
    Picking Transparent Color.
    -------------------------------------------------------------------------
    Move the cursor over the transparent color. 
    There are two color boxes on the right. 
    The top one is the current transparent color 
    and the bottom one is the color of the pixel 
    the cursor is over. When the top color box 
    contains the transparent color - right
    click the mouse.

    If it is difficult to select the pixel you can 
    zoom the image.

    If the image is too large for the viewing area 
    use the scroll bars - or simply hold the left 
    mouse button and drag the image to the 
    required position.

    When you have selected the color press OK.

5. Press the create button. The time required to do 
   this depends on the complexity of the 
   transparent region.
   
6. When it finishes you will be asked to enter the 
   name of the file to store the rgn data in.


How to use your transparency region
-----------------------------------
I have included a demo project look at.
 
1.Insert the rgn file as a "RGN" resource.
2.Insert the bitmap to be used as the background.

3.Insert a dialog
  Set the dialog's border property to none.

4.Insert a picture object into the dialog
  Set its type as a "Bitmap"
  Set the backgournd bitmap as the image.
  (This is a bit annoying as it must be
   the first in the tab order. Otherwise it
   will overwrite your other dialog controls
   when the dialog is displayed. Unfortunately
   it works the other way whilst editing the dialog!
   So just set it to first when you have finished
   the dialog box.)

5.Create the dialog from the template.
  Base the dialog  on the CRGNDlg class.
  Get the rgndlg.cpp and .h files from the
  demo project.

6.In the dialog source:
  In your constructor just add the RGN resource id.
  eg - CRGNDlg(CRgnDemoDlg::IDD, pParent,"BACKGROUND")


Things to look at -
In the rgndlg.cpp :OnInitDialog()
1. Read in the file (or as in the rgndlg load 
   the resource) 
2. Use CreateFromData to create the rgn from the data.
3. Use SetWindowRgn to set the region.


Note 
----
1.The SetWindowRgn must be done before you call the base 
  initdialog function 

2.You cannot use the rgn used in the 
  SetWindowRgn -it belongs to windows - don't 
  even delete it.

3.If the outline dragging is slow use the full invert.
  It isn't as nice but is faster for complex regions.
  Its in Inverttracker in rgndlg.cpp


A Big Note
----------
'Small fonts' / 'Large fonts' problem

The problem:
------------
For example you have "small fonts" selected in your 
desktop display config. You happily create your dialog box , 
put in your bmp and insert all the required controls aligning 
them carefully to the background bmp. You then triumphantly 
send out your app and then get people complaining that 
things don't line up.

The cause:
----------
These people have 'large fonts' selected. 
Windows scales the dialog box and its controls according 
to the font. The font in turn is dependent on the 'small/large' 
font setting - so Ms Sans Serif 8 is not the same 'size'.
Since the image and the rgn are pixel based there is a problem.

The solution:
-------------
There are two possible solutions -
1. Scaling
   This is what I have implemented in the demo app. 
   The drawback of scaling is that it is not exact. 
   E.g. Scaling from an app designed in "small fonts" is a 
   bit blocky if scaled up for "large fonts".

2. Have two separate images and two RGN files.
   A "reliable" way of determing the font mode is
   if (dc.GetDeviceCaps(LOGPIXELSX) == 120) then its large
   else its small  //dc.GetDeviceCaps(LOGPIXELSX)==96
	    
Tis not my fault.

-----------------------------------------------------------------------------
If you would like to e-mail me:
richard@ncoast.lia.net.

