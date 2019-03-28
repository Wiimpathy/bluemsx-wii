
#include "GuiContainer.h"
#include "image_container.inc"

#define TILE_SIZE 12

GuiContainer::GuiContainer( int posx,  int posy, int width, int height, int alpha)
            : TiledLayer((width + TILE_SIZE/2)/TILE_SIZE+1,
                         (height + TILE_SIZE/2)/TILE_SIZE+1, 0)
{
	cols = (width + TILE_SIZE/2) /TILE_SIZE;
	rows = (height + TILE_SIZE/2) / TILE_SIZE;
	image.LoadImage(image_container);
	SetStaticTileset(&image, TILE_SIZE, TILE_SIZE);
    SetTransparency(alpha);
	SetPosition(posx, posy);
	Render();
}

GuiContainer::~GuiContainer()
{
}

void GuiContainer::SetPosition( int posx,  int posy)
{
	TiledLayer::SetPosition(posx - TILE_SIZE, posy - TILE_SIZE);
}

int GuiContainer::GetWidth(void)
{
    return GetColumns() * TILE_SIZE;
}

int GuiContainer::GetHeight(void)
{
    return GetRows() * TILE_SIZE;
}

void GuiContainer::Render(void)
{
	for(int y = 1; y <= rows; y++) {
        for(int x = 1; x <= cols; x++) {
            if( x == 1 && y == 1 )
        		SetCell(x, y, 1);
            else if( x == 1 && y == rows )
        		SetCell(x, y, 7);
            else if( x == cols && y == 1 )
        		SetCell(x, y, 3);
            else if( x == cols && y == rows )
        		SetCell(x, y, 9);
            else if( x == 1 )
        		SetCell(x, y, 4);
            else if( x == cols )
        		SetCell(x, y, 6);
            else if( y == 1 )
        		SetCell(x, y, 2);
            else if( y == rows )
        		SetCell(x, y, 8);
            else
        		SetCell(x, y, 5);
		}
	}
}

