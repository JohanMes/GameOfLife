#include <windows.h>
#include <stdio.h>

#include "resource.h"
#include "map.h"

Map* map = NULL;

LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	switch(Message) {
		case WM_PAINT: {
	
			PAINTSTRUCT ps;
			HDC dc = BeginPaint(hwnd,&ps);
			
			// Teken de map, loop alleen door de gewijzigde dingen
			for(unsigned int i = 0;i < map->redrawlist.size();i++) {
				Block* thisblock = map->redrawlist[i];

				RECT thisrect = {
					map->blocksize*thisblock->x,
					map->blocksize*thisblock->y,
					map->blocksize*(thisblock->x + 1),
					map->blocksize*(thisblock->y + 1)};

				if(thisblock->GetState() == bsAlive) {
					SetBkColor(dc,RGB(255,255,255));
				} else if(thisblock->GetState() == bsDead) {
					SetBkColor(dc,RGB(0,0,0));
				} else if(thisblock->GetState() == bsWall) {
					SetBkColor(dc,RGB(255,0,0));
				} else if(thisblock->GetState() == bsFood) {
					SetBkColor(dc,RGB(0,255,0));
				}
				
				// Deze is veel sneller dan FillRect of Rectangle
				ExtTextOut(dc,0,0,ETO_OPAQUE,&thisrect,"",0,NULL);
			}
			
			EndPaint(hwnd,&ps);
			break;
		}
		case WM_CREATE: {
			ShowWindow(hwnd,SW_MAXIMIZE);
			
			int pixelwidth = 8;
			
			RECT clientsize;
			GetClientRect(hwnd,&clientsize);
			map = new Map(clientsize.right/pixelwidth,clientsize.bottom/pixelwidth,pixelwidth);

			// Lijnen
			for(int i = 0;i < std::min(map->xsize,map->ysize);i++) {
				map->Get(i,i)->SetState(bsAlive);
				map->Get(map->xsize - i - 1,i)->SetState(bsAlive);
			}
			
//			// Lijnen
//			for(int i = 0;i < std::min(map->xsize,map->ysize);i++) {
//				for(int j = 0;j < std::min(map->xsize,map->ysize);j += 1) {
//					map->Get(i + j,i)->SetState(bsAlive);
//				}
//			}
			
//			// Kolonies
//			for(int i = 0;i < (map->xsize * map->ysize) / 128;i++) {
//				int x = RandomRange(0,map->xsize - 1);
//				int y = RandomRange(0,map->ysize - 1);
//				
//				map->AddColony(x,y);
//			}
			
//			// Maak wat voedsel
//			for(int i = 0;i < (map->xsize * map->ysize) / 128;i++) {
//				int x = RandomRange(0,map->xsize - 1);
//				int y = RandomRange(0,map->ysize - 1);
//				
//				map->Get(x,y)->SetState(bsFood);
//			}
			
//			// Maak een paar muurtjes
//			for(int i = 10;i < map->xsize-10;i++) {
//				map->Get(i,map->ysize/2 - 30)->SetState(bsWall);
//				map->Get(i,map->ysize/2)->SetState(bsWall);
//				map->Get(i,map->ysize/2 + 30)->SetState(bsWall);
//			}

			map->Invalidate();
					
			break;
		}
		case WM_DESTROY: {
			PostQuitMessage(0);
			break;
		}
		default:
			return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	WNDCLASSEXA wc;
	MSG Msg;

	memset(&wc,0,sizeof(wc));
	wc.cbSize		 = sizeof(WNDCLASSEX);
	wc.lpfnWndProc	 = WndProc;
	wc.hInstance	 = hInstance;
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszClassName = "WindowClass";
	wc.hIcon		 = LoadIcon(hInstance, "A");
	wc.hIconSm		 = LoadIcon(hInstance, "A");

	if(!RegisterClassEx(&wc)) {
		MessageBox(NULL, "Window Registration Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	hwnd = CreateWindowEx(WS_EX_COMPOSITED,"WindowClass","Game of Life",WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,NULL,hInstance,NULL);
		
	while(Msg.message != WM_QUIT) {
		if(PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		} else {
			
			// Calculate
			if(map->Iterate()) {
				
				// Update caption
				char text[512];
				snprintf(text,512,
					"Game of Life, iteration = %d, frequency = %.1fHz, time = %.3gs, width = %d, height = %d, resolution = %g%%",
					map->iteration,map->fiterate,map->time,map->xsize,map->ysize,100.0f/map->blocksize);
				SetWindowText(hwnd,text);
				
				// Repaint
				RedrawWindow(hwnd,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);
				
			} else { // done!
			
				// Update final caption once
				char text[512];
				snprintf(text,512,
					"Game of Life, iteration = %d, time = %.3gs, width = %d, height = %d",
					map->iteration,map->time,map->xsize,map->ysize);
				SetWindowText(hwnd,text);
				
				break; // break from greedy loop
			}
		}
	}
	
	if(Msg.message != WM_QUIT) { // stop

		// If we end up here, we don't need all the time we can get anymore
		while(GetMessage(&Msg, NULL, 0, 0) > 0) {
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}
		
	}
	return Msg.wParam;
}
