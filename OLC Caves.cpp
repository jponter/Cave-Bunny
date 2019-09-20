//dev branch

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"



#define OLC_PGE_GRAPHICS2D
#include "olcPGEX_Graphics2D.h"

#define OLC_PGEX_SOUND
#include "olcPGEX_Sound.h"

#include "cAnimator.h"


using namespace std;

//#define _debug


//world building values
unsigned int seed = 2;
int nLevelGenerations = 4;





// Override base class with your custom functionality
class Example : public olc::PixelGameEngine
{
public:
	Example()
	{
		sAppName = "Cave Bunny";
	}


private:
	wstring sLevel;
int nLevelWidth;
int nLevelHeight;

float fCameraPosX = 0.0f;
float fCameraPosY = 0.0f;

float fPlayerPosX = 4.0f;
float fPlayerPosY = 4.0f;

float fPlayerVelX = 0.0f;
float fPlayerVelY = 0.0f;

float fPlayerFrame = 0;

float fFaceDir = +1.0f;

float fBackdropScaleX = 10.0f;
float fBackdropScaleY = 10.0f;

bool bPlayerOnGround = false;
bool bPlayerRight = true;
bool noclip = false;

olc::Sprite *sprFloor;
olc::Sprite *sprFloor2;
olc::Sprite *sprFloor3;

//olc::AnimatedSprite* sprPlayerIdleR;
olc::Sprite *sprPlayerLeft;
olc::Sprite *sprPlayerRight;
olc::Sprite* sprPlayerJumpLeft;
olc::Sprite* sprPlayerJumpRight;
olc::Sprite* sprPlayer;
olc::Sprite* sprBackground;
olc::Sprite* sprGem;

olc::Sprite* minimap;
olc::Sprite* minimap2;

int totalgems = 0;
int gems = 0;

olc::cAnimator animPlayer;
//TMXLoader* loader = new TMXLoader();


vector <unsigned int> leveldata;
vector <unsigned int> cavedata;
vector <unsigned int> tmpcave;

int sndPickup;
int sndJump;



public:
	bool OnUserCreate() override
	{
		// Called once at the start, so create things here

		nLevelWidth = 100;
		nLevelHeight = 32;


		//some sprite tiles
		sprFloor = new olc::Sprite("sprites/tile_00.png");
		sprFloor2 = new olc::Sprite("sprites/tile_02.png");
		sprFloor3 = new olc::Sprite("sprites/tile_03.png");

		//background sprite 
		sprBackground = new olc::Sprite("sprites/bg3.png");

		// gem sprite
		sprGem = new olc::Sprite("sprites/gem.png");

		
		// need to implement some error coding for the rest of the sprites
		if (sprFloor->height == 0) {
			cout << "error loading sprite";
			exit(10);
		}
		
		// minimap sprite holders
		minimap = new olc::Sprite(nLevelWidth, nLevelHeight);
		minimap2 = new olc::Sprite(nLevelWidth, nLevelHeight);
		
		// set my player to idle
		animPlayer.ChangeState("idle");

		// load my sprites into the animation player objects
		animPlayer.hLoadSprite("idle", "sprites/idle right 4.png" , 4, 32, 32);
		animPlayer.hLoadSprite("walk","sprites/walk right 6.png", 6, 32, 32);
		animPlayer.hLoadSprite("jump", "sprites/jump right 8.png", 8, 32, 32);

		//sound initialise
		olc::SOUND::InitialiseAudio();

		// load the wav files for the sound effects
		sndPickup = olc::SOUND::LoadAudioSample("wav/Pickup_Coin18.wav");
		sndJump = olc::SOUND::LoadAudioSample("wav/Jump10.wav");
		

		
		
		

		//create the level
		cavedata.resize(nLevelWidth * nLevelHeight);
		//set the seed which is saved up in the global variables - lazy :(
		srand(seed); 
		int chance = 35; // 35% chance there will be a block - move to a user defined variable at some point

		for (int x = 0; x < nLevelWidth; x++)
		{
			for (int y = 0; y < nLevelHeight; y++)
			{
				if ((rand() % 100) < chance)
				{
					cavedata[y * nLevelWidth + x ] = 1;
				}
			}
		}

		
		//leveldata = new (unsigned int)[nLevelWidth * nLevelHeight];
		leveldata.resize(nLevelWidth * nLevelHeight);
		tmpcave.resize(nLevelWidth * nLevelHeight);


		// loop through the world simulations - update the cellular automata world
		for (int i = 0; i < nLevelGenerations; i++) {
			SimulationUpdate();
		}
		

		//copy over the resulting cavedata world generated in the simulation to the leveldata
		leveldata = cavedata;

		

		// sprinkle some treasure
		SprinkleTreasure();


		//create the minimap - call a function to do this
		
		UpdateMiniMap();
		// TODO:
		// go through the level and check the neighbour at y-1; if it's a block then change the ID of x,y to 2
		for (int x = 0; x < nLevelWidth; x++)
		{
			for (int y = 1; y < nLevelHeight; y++) // we dont need the top layer
			{

				if (leveldata[((y-1) * nLevelWidth) + x] == 1)
				{
					leveldata[(y * nLevelWidth) + x] = 4;

				}
				
			}
		}



		//done with my setup - return
		return true;
	}

	bool OnUserDestroy()
	{
		//kill the sound engine
		olc::SOUND::DestroyAudio();
		return true;
	}

	void UpdateMiniMap()
	{

		//using the minimap sprite take a copy of the leveldata 1px x 1px and map directly to the minimap sprite
		SetDrawTarget(minimap);
		Clear(olc::CYAN);

		for (int x = 0; x < nLevelWidth; x++)
		{
			for (int y = 0; y < nLevelHeight; y++)
			{

				if (leveldata[y * nLevelWidth + x] == 1 || leveldata[y * nLevelWidth + x] == 4 )
				{
					Draw(x, y, olc::BLACK);

				}
				if (leveldata[y * nLevelWidth + x] == 100)
				{
					Draw(x, y, olc::YELLOW);
				}
			}
		}
		//releast the draw target back to default
		SetDrawTarget(nullptr);
	}

	void SprinkleTreasure()
	{
		// just randomly place some gems whereever there are more than x neighbours (x = treasureHiddenLimit)
		int treasureHiddenLimit = 3;
		for (int x = 0; x < nLevelWidth; x++)
		{
			for (int y = 0; y < nLevelHeight; y++)
			{

				if (leveldata[y * nLevelWidth + x] == 0) //empty space
				{
					int nbs = CountNeighbours(x, y);
					if (nbs > treasureHiddenLimit)
					{
						leveldata[y * nLevelWidth + x] = 100;
						totalgems++;
						/*cout << "Treasure added ";
						cout << x;
						cout << " ";
						cout << y;
							cout << " \n";*/
					}

				}
			}
		}
		
	}


	void SimulationUpdate()
	{
		// standard cellular automata rules to update the level, using a copy of the cavedata so we dont corrupt the world before it's finished updating
		int deathLimit = 3;
		int birthLimit = 4;
		for (int x = 0; x < nLevelWidth; x++)
		{
			for (int y = 0; y < nLevelHeight; y++)
			{
				int nbs = CountNeighbours(x, y);
				if (cavedata[y * nLevelWidth + x] == 1)
				{
					if (nbs < deathLimit)
					{
						tmpcave[y * nLevelWidth + x] = 0;
					}
					else
					{
						tmpcave[y * nLevelWidth + x] = 1;
					}
				}
				else
				{
					if (nbs > birthLimit)
					{
						tmpcave[y * nLevelWidth + x] = 1;
					}
					else
					{
						tmpcave[y * nLevelWidth + x] = 0;
					}
				}
			}
		}

		// done with the temporary cave data, move that over to the actual cave  - this function is called many times so tmpcave is released for next run through
		cavedata = tmpcave;
		
		
		
	}

	int CountNeighbours(int x, int y)
	{
		// given x, y coors - return the number of surrounding cells 9x9 grid (centre doesnt count)
		int count = 0;
		for (int i = -1; i < 2; i++)
		{
			for (int j = -1; j < 2; j++)
			{
				int neighbour_x = x + i;
				int neighbour_y = y + j;

				if (i == 0 && j == 0)
				{
					//do nothing this is the middle
				
				}
				else if (neighbour_x < 0 || neighbour_y < 0 || neighbour_x >= nLevelWidth || neighbour_y >= nLevelHeight)
				{
					count++;
				}
				else if (cavedata[neighbour_y*nLevelWidth + neighbour_x] == 1)
				{
					count++;
				}
			}
		}
		return count;

	}



	bool OnUserUpdate(float fElapsedTime) override
	{

		auto GetTile = [&](int x, int y)
		{
			if (x >= 0 && x < nLevelWidth && y >= 0 && y < nLevelHeight)
				//return sLevel[y * nLevelWidth + x];
				//return (loader->getMap("testmap")->getTileLayer("Tile Layer 1")->getTileVector()[y][x]);
				return (unsigned int)(leveldata[y * nLevelWidth + x]);
			else
				return (unsigned int) 999;
		};

		auto SetTile = [&](int x, int y, unsigned int c)
		{
			if (x >= 0 && x < nLevelWidth && y >= 0 && y < nLevelHeight)
			{
				//sLevel[y * nLevelWidth + x] = c;
				//loader->
				leveldata[y * nLevelWidth + x] = c;
			}
		};

		// Handle Input
		

		if (IsFocused())
		{

			if (GetKey(olc::Key::UP).bHeld)
			{
				fPlayerVelY = -6.0f;
			}

			if (GetKey(olc::Key::DOWN).bHeld)
			{
				fPlayerVelY = 6.0f;
			}

			if (GetKey(olc::Key::LEFT).bHeld)
			{
				fPlayerVelX += (bPlayerOnGround ? -14.0f : -8.0f) * fElapsedTime;
				fFaceDir = -1.0f;
			}

			if (GetKey(olc::Key::RIGHT).bHeld)
			{
				fPlayerVelX += (bPlayerOnGround ? 14.0f : 8.0f) * fElapsedTime;
				fFaceDir = +1.0f;
			}

			if (GetKey(olc::Key::SPACE).bPressed)
			{
				//SimulationUpdate();
				if (fPlayerVelY == 0)
				{
					fPlayerVelY = -10.0f;
					olc::SOUND::PlaySample(sndJump);
					
				}
			}
		

		}

		//add gravity 
		if (!noclip)
		{
			fPlayerVelY += 20.0f * fElapsedTime;
		}

		// add drag for player on the ground
		if (bPlayerOnGround)
		{
			fPlayerVelX += -5.0f * fPlayerVelX * fElapsedTime;
			if (abs(fPlayerVelX) < 0.01f)
			{
				fPlayerVelX = 0.0f;
				animPlayer.ChangeState("idle");
			}
			else
				animPlayer.ChangeState("walk");
		}
		else
		{
			if (fPlayerVelY < 0)
				animPlayer.ChangeState("jump");
		}

		


		float fNewPlayerPosX = fPlayerPosX + fPlayerVelX * fElapsedTime;
		float fNewPlayerPosY = fPlayerPosY + fPlayerVelY * fElapsedTime;

		//clamp velocity

		

		if (fPlayerVelX > 10.0f)
			fPlayerVelX = 10.0f;

		if (fPlayerVelX < -10.0f)
			fPlayerVelX = -10.0f;

		if (fPlayerVelY > 100.0f)
			fPlayerVelY = 100.0f;

		if (fPlayerVelY < -100.0f)
			fPlayerVelY = -100.0f;


		// check for collisions 

		if (!noclip) {

			//check for collectables - definitely a better way of doing this exists - needs to be optimised as checking some areas twice
			
			if (fPlayerVelX <= 0)
			{
				//top left
				if (GetTile(fNewPlayerPosX + 0.0f, fPlayerPosY + 0.0f) == 100) //|| GetTile(fNewPlayerPosX + 0.0f, fPlayerPosY + 0.9f) == 100)
				{
					gems++;
					SetTile(fNewPlayerPosX, fPlayerPosY, 0);
					olc::SOUND::PlaySample(sndPickup);
				}
				else if (GetTile(fNewPlayerPosX + 0.0f, fPlayerPosY + 0.9f) == 100)
				{
					gems++;
					SetTile(fNewPlayerPosX, fPlayerPosY+0.9, 0);
					olc::SOUND::PlaySample(sndPickup);
				}

			}
			else
			{
				//top right
				if (GetTile(fNewPlayerPosX + 1.0f, fPlayerPosY + 0.0f) == 100) // || GetTile(fNewPlayerPosX + 1.0f, fPlayerPosY + 0.9f) == 100)
				{
					gems++;
					SetTile(fNewPlayerPosX+1.0f, fPlayerPosY, 0);
					olc::SOUND::PlaySample(sndPickup);
				}
				else if (GetTile(fNewPlayerPosX + 1.0f, fPlayerPosY + 0.9f) == 100)
				{
					gems++;
					SetTile(fNewPlayerPosX + 1.0f, fPlayerPosY+0.9f, 0);
					olc::SOUND::PlaySample(sndPickup);
				}
				 
			}


			
			if (fPlayerVelY <= 0)
			{
				// bottom left
				if (GetTile(fPlayerPosX + 0.0f, fNewPlayerPosY + 0.0f) == 100)
				{
					gems++;
					SetTile(fPlayerPosX, fNewPlayerPosY, 0);
					olc::SOUND::PlaySample(sndPickup);
				}
				else if (GetTile(fPlayerPosX + 0.9f, fNewPlayerPosY + 0.0f) == 100)
				{

					gems++;
					SetTile(fPlayerPosX +0.9f, fNewPlayerPosY, 0);
					olc::SOUND::PlaySample(sndPickup);
				}	

			}
			else
			{
				if (GetTile(fPlayerPosX + 0.0f, fNewPlayerPosY + 1.0f) == 100 )
				{
					gems++;
					SetTile(fPlayerPosX, fNewPlayerPosY+1.0f, 0);
					olc::SOUND::PlaySample(sndPickup);
				}
				else if (GetTile(fPlayerPosX + 0.9f, fNewPlayerPosY + 1.0f) == 100)
				{
					gems++;
					SetTile(fPlayerPosX+0.9f , fNewPlayerPosY + 1.0f, 0);
					olc::SOUND::PlaySample(sndPickup);
				}
			}

			
			
				
			
			
			// check for world collisions

			if (fPlayerVelX <= 0)
			{
				
				if (GetTile(fNewPlayerPosX + 0.0f, fPlayerPosY + 0.0f) != 0 || GetTile(fNewPlayerPosX + 0.0f, fPlayerPosY + 0.9f) != 0)
				{
					fNewPlayerPosX = (int)fNewPlayerPosX + 1;
					fPlayerVelX = 0;
				}
			}
			else
			{
				if (GetTile(fNewPlayerPosX + 1.0f, fPlayerPosY + 0.0f) != 0 || GetTile(fNewPlayerPosX + 1.0f, fPlayerPosY + 0.9f) != 0)
				{
					fNewPlayerPosX = (int)fNewPlayerPosX;
					fPlayerVelX = 0;
				}
			}


			bPlayerOnGround = false;
			if (fPlayerVelY <= 0)
			{
				if (GetTile(fPlayerPosX + 0.0f, fNewPlayerPosY + 0.0f) != 0 || GetTile(fPlayerPosX + 0.9f, fNewPlayerPosY + 0.0f) != 0)
				{
					fNewPlayerPosY = (int)fNewPlayerPosY + 1;
					fPlayerVelY = 0;
				}
			}
			else
			{
				if (GetTile(fPlayerPosX + 0.0f, fNewPlayerPosY + 1.0f) != 0 || GetTile(fPlayerPosX + 0.9f, fNewPlayerPosY + 1.0f) != 0)
				{
					fNewPlayerPosY = (int)fNewPlayerPosY;
					fPlayerVelY = 0;
					bPlayerOnGround = true;
				}
			}
		}
		



			fPlayerPosX = fNewPlayerPosX;
			fPlayerPosY = fNewPlayerPosY;
		

		// check for out of bounds player in the <0 area and clamp if needed
		if (fPlayerPosX <= 0) {
			fPlayerPosX = 0;
			fPlayerVelX = 0;
		}
		if (fPlayerPosY <= 0) {
			fPlayerPosY = 0;
			fPlayerVelY = 0;
		}


				
		fCameraPosX = fPlayerPosX;
		fCameraPosY = fPlayerPosY;





		//Draw Level

		int nTileWidth = 32;
		int nTileHeight = 32;
		int nVisibleTilesX = ScreenWidth() / nTileWidth;
		int nVisibleTilesY = ScreenHeight() / nTileHeight;

		float fOffsetX = fCameraPosX - (float)nVisibleTilesX / 2.0f;
		float fOffsetY = fCameraPosY - (float)nVisibleTilesY / 2.0f;


		//clamp camera to game boundaries
		if (fOffsetX < 0) fOffsetX = 0;
		if (fOffsetY < 0) fOffsetY = 0;

		if (fOffsetX > nLevelWidth - nVisibleTilesX) fOffsetX = nLevelWidth - nVisibleTilesX;
		if (fOffsetY > nLevelHeight - nVisibleTilesY) fOffsetY = nLevelHeight - nVisibleTilesY;

		// get some offsets for smooth movement
		float fTileOffsetX = (fOffsetX - (int)fOffsetX) * nTileWidth;
		float fTileOffsetY = (fOffsetY - (int)fOffsetY) * nTileHeight;

		Clear(olc::BLACK);

		fBackdropScaleX = (float)(sprBackground->width - ScreenWidth()) / (float)((nLevelWidth) + (float)nVisibleTilesX);
		fBackdropScaleY = (float)(sprBackground->height - ScreenHeight()) / (float)((nLevelHeight) + (float)nVisibleTilesY);
		DrawPartialSprite(0, 0, sprBackground, fOffsetX* fBackdropScaleX, fOffsetY* fBackdropScaleY, ScreenWidth(), ScreenHeight());

		//draw the tile map
		for (int x = -1; x < nVisibleTilesX +1 ; x++)
		{
			for (int y = -1; y < nVisibleTilesY +1; y++)
			{
				//float tempx = (int)((((float)x - fOffsetX) * (float(nTileWidth))));
				int tempy = y * nTileHeight - fTileOffsetY;
				unsigned int sTileID = GetTile(x + fOffsetX, y + fOffsetY);
				switch (sTileID)
				{
				case 0:
					//FillRect(x * nTileWidth -(fTileOffsetX), y * nTileHeight -fTileOffsetY, nTileWidth ,  nTileHeight , olc::CYAN);
					//DrawRect(tempx,tempy,(x+1) * nTileWidth - fTileOffsetX, (y+1) * nTileHeight - fOffsetY, olc::WHITE);
					break;
				case 1:
					//FillRect(x * nTileWidth - (fTileOffsetX), y * nTileHeight - fTileOffsetY, nTileWidth, nTileHeight, olc::WHITE);
					//DrawRect(x* nTileWidth - fTileOffsetX, y* nTileHeight - fTileOffsetY, nTileWidth, nTileHeight, olc::RED);
					DrawSprite(x* nTileWidth - ceil(fTileOffsetX), y* nTileHeight - fTileOffsetY,sprFloor);
					//DrawSprite(tempx + 0.5f - (tempx < 0.0f), y* nTileHeight - fTileOffsetY, sprFloor);
					//DrawRect(tempx , tempy, nTileWidth, nTileHeight, olc::RED);
					break;
				case 3:
					DrawSprite(x * nTileWidth - ceil(fTileOffsetX), y * nTileHeight - fTileOffsetY, sprFloor);
					break;

				case 4:
					DrawSprite(x* nTileWidth - ceil(fTileOffsetX), y* nTileHeight - fTileOffsetY, sprFloor2);
					break;

				case 5:
					DrawSprite(x* nTileWidth - ceil(fTileOffsetX), y* nTileHeight - fTileOffsetY, sprFloor3);
					break;

				case 100:
					//gem
					SetPixelMode(olc::Pixel::MASK);
					DrawSprite(x* nTileWidth - ceil(fTileOffsetX), y* nTileHeight - fTileOffsetY, sprGem);
					SetPixelMode(olc::Pixel::NORMAL);
					break;




				default:
					
					break;
				}
			}
		}


		

			//// Draw the player
			SetPixelMode(olc::Pixel::MASK);
			animPlayer.Update(fElapsedTime);
			olc::GFX2D::Transform2D t;
			t.Translate(-16, -16); // translate sprite so middle of sprite is at 0,0 world space
			t.Scale(fFaceDir * 1.0f, 1.0f); // scale in the x direction essentially flipping the sprite if fFaceDir is negative 

			t.Translate((fPlayerPosX - fOffsetX) * nTileWidth + 16, (fPlayerPosY - fOffsetY) * nTileHeight + 16); //translate to correct world position again

			////experimental bounding box																								 
			////DrawRect(((fPlayerPosX - fOffsetX ) *nTileWidth)+6, (fPlayerPosY - fOffsetY) *nTileHeight, 32-12, nTileHeight, olc::RED);

			animPlayer.DrawSelf(this, t);


			//HUD
			SetPixelMode(olc::Pixel::NORMAL);
			SetPixelBlend(1.0f);
			std::string HUD1 = "Gems = " + std::to_string(gems) + " / " + std::to_string(totalgems);
			DrawString(6, 7, HUD1, olc::BLACK);
			DrawString(5, 6, HUD1, olc::YELLOW);


			// draw the minimap
			UpdateMiniMap();
			SetPixelMode(olc::Pixel::ALPHA);
			SetPixelBlend(0.4f);
			SetDrawTarget(minimap2);
			DrawSprite(0, 0, minimap);
			Draw(fPlayerPosX, fPlayerPosY, olc::RED);
			SetDrawTarget(nullptr);


			DrawSprite(ScreenWidth()-nLevelWidth -5, 5 , minimap2);
			SetPixelMode(olc::Pixel::NORMAL);

			

#ifdef _debug
			std::string sDebug1 = "fPlayerPosX " + std::to_string(fPlayerPosX) + " fPlayerPosY " + std::to_string(fPlayerPosY);
			std::string sDebug2 = "fOffsetX " + std::to_string(fOffsetX) + " fOffsetY " + std::to_string(fOffsetY);
			std::string sDebug3 = "fTileOffsetX " + std::to_string(floor(fTileOffsetX)) + " fTileOffsetY " + std::to_string(fTileOffsetY);
			std::string sDebug4 = "fPlayerFrame " + std::to_string(int(fPlayerFrame)) + " fPLayerVelX " + std::to_string(fPlayerVelX);

			DrawString(0, 0, sDebug1, olc::BLACK);
			DrawString(0, 10, sDebug2, olc::BLACK);
			DrawString(0, 20, sDebug3, olc::BLACK);
			DrawString(0, 30, sDebug4, olc::BLACK);

#endif
		
		
		//cout << "X: ";
		//cout << fPlayerPosX;
		//cout << "XOff: ";
		//cout << fOffsetX;
		//cout << "Calc: ";
		//out << fPlayerPosX + 1 - fOffsetX * nTileWidth;
		//cout << "\n";


		return true;
	}
};







int main()
{
	Example demo;
	if (demo.Construct(320,256 , 4, 4))
		demo.Start();
	return 0;
}