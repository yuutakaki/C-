#define _CRT_SEURE_NO_WARNINGS
#pragma warning(disable:4996)

#include <cstdio>  
#include <iostream>  
#include <cstring>
#include <string.h>

#define FILE_HEADER_SIZE 14                // ファイルヘッダのサイズ  
#define INFO_HEADER_SIZE 40                // 情報ヘッダのサイズ  

using namespace std;

/*
* ファイルヘッダー構造体
*/
typedef struct FileHeader 
{
	uint8_t data[FILE_HEADER_SIZE];  // 加工前データ（書き出しのため必要）  
	string fileType;                 // ファイルタイプ  
	int fileSize;                    // ファイルサイズ  
} FileHeader;

/*
* 情報ヘッダー構造体
*/
typedef struct InfoHeader 
{
	uint8_t data[INFO_HEADER_SIZE];  // 加工前データ（書き出しのため必要）  
	int infoHeaderSize;              // 情報ヘッダのサイズ  
	int width;                       // 画像の幅  
	int height;                      // 画像の高さ  
	int clrPerPixel;                 // 1ピクセル当たりの色数  
	int dataSize;                    // 画像データのサイズ  
} InfoHeader;

/*
* ピクセルの色情報構造体
*/
typedef struct Color 
{
	int r;                // 赤成分  
	int g;                // 緑成分  
	int b;                // 青成分  
} Color;

int bit2Integer(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4);

/*
* ビットマップ処理クラス
*/
class BitMapProcessor {
	FILE* bmp;               // ビットマップのファイルポインタ  
	uint8_t* img;            // ビットマップデータ（加工用）  
	uint8_t* org;            // ビットマップデータ（読み込み時）  
	FileHeader fHeader;      // ファイルヘッダ  
	InfoHeader iHeader;      // 情報ヘッダ  

public:
	BitMapProcessor() {
		bmp = NULL;
		img = NULL;
		org = NULL;
	};

	~BitMapProcessor() 
	{
		if(bmp != NULL)
		{
			fclose(bmp);
		}
		delete[]img;
		delete[]org;
	}
	void loadData(string filename);
	void dispBmpInfo();
	void writeData(string filename,int n);
	Color getColor(int row, int col);
	void setColor(int row, int col, int r, int g, int b);
	void restore();
	int height() { return iHeader.height; };
	int width() { return iHeader.width; };
private:
	void readFileHeader();
	void readInfoHeader();
	void readBmpData();

};

/*
* 4ビット情報をInt整数値に変換
*/
int bit2Integer(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4) 
{
	return b1 +
		b2 * 256 +
		b3 * 256 * 256 +
		b4 * 256 * 256 * 256;
}

/*
* ビットマップデータをロードする
*/
void BitMapProcessor::loadData(string filename) 
{
	if (bmp != NULL)
	{
		fclose(bmp);
	}
		
	bmp = fopen(filename.c_str(), "rb");
	if (bmp == NULL)
	{
		printf("ファイルオープンに失敗しました。\n");
	}
	
		
	readFileHeader();
	readInfoHeader();
	readBmpData();
}

/*
* ファイルヘッダを読む
*/
void BitMapProcessor::readFileHeader() 
{
	uint8_t data[FILE_HEADER_SIZE];
	size_t size = fread(data, sizeof(uint8_t), FILE_HEADER_SIZE, bmp);

	memcpy(fHeader.data, data, sizeof(data));
	fHeader.fileType = "";
	fHeader.fileType += data[0];
	fHeader.fileType += data[1];
	fHeader.fileSize = bit2Integer(data[2], data[3], data[4], data[5]);
}

/*
* 情報ヘッダを読む
*/
void BitMapProcessor::readInfoHeader() 
{
	uint8_t data[INFO_HEADER_SIZE];
	size_t size = fread(data, sizeof(uint8_t), INFO_HEADER_SIZE, bmp);

	memcpy(iHeader.data, data, sizeof(data));
	iHeader.infoHeaderSize = bit2Integer(data[0], data[1], data[2], data[3]);
	iHeader.width = bit2Integer(data[4], data[5], data[6], data[7]);
	iHeader.height = bit2Integer(data[8], data[9], data[10], data[11]);
	iHeader.clrPerPixel = bit2Integer(data[14], data[15], 0, 0);
	iHeader.dataSize = bit2Integer(data[20], data[21], data[22], data[23]);
}

/*
* 画像データを読む
*/
void BitMapProcessor::readBmpData()
{
	if (img != NULL)
	{
		delete[]img;
	}

	int sz = iHeader.dataSize;
	img = new uint8_t[sz];
	size_t size = fread(img, sizeof(uint8_t), sz, bmp);
	if (size != sz)
	{
		printf("画像データ読み込みのサイズが矛盾しています。");
	}

	// バックアップ用にorgに画像データをコピー  
	if (org != NULL)
	{
		delete[]org;
	}
	org = new uint8_t[sz];
	memcpy(org, img, sz);
}

/*
* ビットマップ情報の表示（デバッグ用）
*/
void BitMapProcessor::dispBmpInfo() 
{
	cout << "■ファイルヘッダ情報" << endl;
	cout << "ファイルタイプ: " << fHeader.fileSize << endl;
	cout << "ファイルサイズ: " << fHeader.fileSize << endl;

	cout << "■情報ヘッダ情報" << endl;
	cout << "情報ヘッダサイズ: " << iHeader.infoHeaderSize << endl;
	cout << "画像幅: " << iHeader.width << endl;
	cout << "画像高: " << iHeader.height << endl;
	cout << "１ピクセルあたりの色数: " << iHeader.clrPerPixel << endl;
	cout << "画像データのサイズ: " << iHeader.dataSize << endl;
}

/*
* ビットマップデータのファイル書き出し
*/
void BitMapProcessor::writeData(string filename, int n)
{
	char fname[100];

	sprintf(fname,"%s%d.bmp", filename.c_str(), n);

	FILE* out = fopen(fname, "wb");

	if (out == NULL)
	{
		printf("書き出し先のファイルを開けません。\n");
	}

	fwrite(fHeader.data, sizeof(uint8_t), FILE_HEADER_SIZE, out);
	fwrite(iHeader.data, sizeof(uint8_t), INFO_HEADER_SIZE, out);
	fwrite(img, sizeof(uint8_t), iHeader.dataSize, out);

	fclose(out);
}

/*
* 指定されたピクセルの色を取得
*/
Color BitMapProcessor::getColor(int row, int col) 
{
	if (row < 0 || row >= iHeader.height)
	{
		printf("getColor(): rowが範囲外です。\n");
	}
	if (col < 0 || col >= iHeader.width)
	{
		printf("getColor(): colが範囲外です。\n");
	}

	int width = 3 * iHeader.width;
	while (width % 4)        // ビットマップの1列は4の倍数ビットからなる  
		++width;

	int bPos = row * width + 3 * col;
	int gPos = bPos + 1;
	int rPos = bPos + 2;

	Color color;
	color.r = img[rPos];
	color.g = img[gPos];
	color.b = img[bPos];

	return color;
}

/*
* 指定されたピクセルに色を設定
*/
void BitMapProcessor::setColor(int row, int col, int r, int g, int b) 
{
	if (row < 0 || row >= iHeader.height)
	{
		printf("getColor(): rowが範囲外です。\n");
	}

	if (col < 0 || col >= iHeader.width)
	{
		printf("getColor(): colが範囲外です。\n");
	}		

	int width = 3 * iHeader.width;
	while (width % 4)        // ビットマップの1列は4の倍数ビットからなる  
		++width;

	int bPos = row * width + 3 * col;
	int gPos = bPos + 1;
	int rPos = bPos + 2;

	img[rPos] = r;
	img[gPos] = g;
	img[bPos] = b;
}

/*
* ビットマップデータを加工前に復元する
*/
void BitMapProcessor::restore() {
	memcpy(img, org, iHeader.dataSize);
}


/*
* テスト用関数（1）モノクロ化
*/
void twoTone(BitMapProcessor* bmp)
{
	for (int i = 0; i < bmp->height(); i++)
	{
		for (int j = 0; j < bmp->width(); j++)
		{
			int ave = 0;
			ave += bmp->getColor(i, j).r;
			ave += bmp->getColor(i, j).g;
			ave += bmp->getColor(i, j).b;
			ave /= 3;

			bmp->setColor(i, j, ave, ave, ave);
		}
	}
}

/*
* テスト関数（2）指定範囲の切り取り
*/
void extractArea(BitMapProcessor* bmp) 
{
	int r0;
	int r1;
	int c0;
	int c1;

	printf("切り取り範囲の指定をしてください\n");
	printf("縦方向を決めます。\n");
	printf("左下の縦座標を入力してください\n");
	scanf("%d", &r0);
	printf("縦方向の高さを入力してください\n");
	scanf("%d", &r1);
	printf("左下の横座標を入力してください\n");
	scanf("%d", &c0);
	printf("横方向の入力してください\n");
	scanf("%d", &c1);


	for (int i = 0; i < bmp->height(); i++)
	{
		for (int j = 0; j < bmp->width(); j++)
		{
			if (r0 <= i && i <= r1 && c0 <= j && j <= c1)
				continue;
			bmp->setColor(i, j, 255, 255, 255);
		}
	}
}

/*
* テスト関数（3） 色の反転
*/
void invert(BitMapProcessor* bmp) 
{
	for (int i = 0; i < bmp->height(); i++)
	{
		for (int j = 0; j < bmp->width(); j++)
		{
			int ave = 0;
			int r = bmp->getColor(i, j).r;
			int g = bmp->getColor(i, j).g;
			int b = bmp->getColor(i, j).b;

			bmp->setColor(i, j, 255 - r, 255 - g, 255 - b);
		}
	}
}

/*
* テスト関数（4）モザイク化
*/
void mosaic(BitMapProcessor* bmp, int level)
{
	double size = 0;
	double size1 = 0;

	printf("縦サイズの入力をしてください\n");
	scanf("%lf", &size);
	printf("横のサイズを入力してください\n");
	scanf("%lf", &size1);

	if (level <= 0)
		level = 1;
	

	for (int i = 0; i < bmp->height(); i += size * level)
	{
		for (int j = 0; j < bmp->width(); j += size1 * level)
		{
			int r = 0;
			int g = 0;
			int b = 0;
			int cnt = 0;

			for (int x = -level; x <= level; x++)
			{
				for (int y = -level; y <= level; y++)
				{
					int xt = i + x;
					int yt = j + y;

					if (xt < 0 || yt < 0 || xt >= bmp->height() || yt >= bmp->width())
						continue;
					++cnt;
					r += bmp->getColor(xt, yt).r;
					g += bmp->getColor(xt, yt).g;
					b += bmp->getColor(xt, yt).b;
				}
			}

			r /= cnt;
			g /= cnt;
			b /= cnt;

			for (int x = -level; x <= level; x++)
			{
				for (int y = -level; y <= level; y++)
				{
					int xt = i + x;
					int yt = j + y;

					if (xt < 0 || yt < 0 || xt >= bmp->height() || yt >= bmp->width())
						continue;

					bmp->setColor(xt, yt, r, g, b);
				}
			}
		}
	}
}

//テスト用関数(5) 拡大

void Shrink(BitMapProcessor* bmp) 
{
	int i, j, k, l, w, h, count,a;
	unsigned long tmp_r, tmp_g, tmp_b;

	//bmp->height() / a + ((bmp->height() % a == 0) ? 0 : 1);
	//bmp->width() / a + ((bmp->width() % a == 0) ? 0 : 1);
	printf("縮小の倍率を指定してください\n");
	scanf("%d", &a);
	
	for (i = 0; i < bmp->height(); i++)
	{
		h = (a * (i + 1) <= bmp->height()) ? a : bmp->height() - a * i;
		for (j = 0; j < bmp->width(); j++)
		{
			w = (a * (j + 1) <= bmp->width()) ? a : bmp->width() - a * j;
			tmp_r = 0; tmp_g = 0; tmp_b = 0; count = 0;
			for (k = 0; k < h; k++)
			{
				for (l = 0; l < w; l++)
				{
					tmp_r += bmp->getColor(i * a + k, j * a + l).r;
					tmp_g += bmp->getColor(i * a + k, j * a + l).g;
					tmp_b += bmp->getColor(i * a + k, j * a + l).b;
					count++;
				}
			}
			bmp->setColor(i,j,tmp_r, tmp_g, tmp_b);
		}
	}
}

/*
* メイン処理
*/
int main() 
{
	BitMapProcessor bmp[100];
	char filename[256];
	char syori[16];
	int cnt = 0,j=0;

	while (1)
	{
		printf("ファイル名を入力してください\n");
		printf("nextで処理に進む\n");
		scanf("%s", filename);

		if (strcmp(filename, "next") == 0)
		{
			break;
		}
		else
		{
			// ビットマップデータのロード  
			bmp[cnt].loadData(filename);

			// ビットマップ情報の表示  
			bmp[cnt].dispBmpInfo();

			cnt++;
		}

		
	}
		
	
	printf("処理を決めてください\n");
	printf("mono 画像のモノクロ化\n");
	printf("kiri 画像の切り取り\n");
	printf("hanten 画像の色の反転\n");
	printf("moza 画像のモザイク化\n");
	printf("shrink 画像の縮小\n");
	printf("exit 終了\n");


	scanf("%s", syori);

	while (1)
	{
		for (int i = 0; i < strlen(syori); i++)
			syori[i]
			=
			tolower(syori[i]);


		if (strcmp(syori, "exit") == 0)
		{
			break;
		}
		
		else if (strcmp(syori, "mono") == 0)
		{
			for (j = 0; j < cnt; j++)
			{
				// テスト1. モノクロ化  
				twoTone(&bmp[j]);
				bmp[j].writeData("mono",j);
				bmp[j].restore();
			}
	
		}
		else if (strcmp(syori, "kiri") == 0)
		{
			for (j = 0; j < cnt; j++)
			{
				// テスト2. 指定範囲の切り出し  
				extractArea(&bmp[j]);
				bmp[j].writeData("kiri",j);
				bmp[j].restore();
			}
		
		}
		else if (strcmp(syori, "hanten") == 0)
		{
			for (j = 0; j < cnt; j++)
			{
				invert(&bmp[j]);
				bmp[j].writeData("hanten",j);
				bmp[j].restore();
			}
	
		}
		else if (strcmp(syori, "moza") == 0)
		{
			for (j = 0; j < cnt; j++)
			{
				// テスト4. モザイク  
				mosaic(&bmp[j], 30);
				bmp[j].writeData("moza",j);
				bmp[j].restore();
			}
		}
		else if (strcmp(syori, "shrink") == 0)
		{
			for (j = 0; j < cnt; j++)
			{
				Shrink(&bmp[j]);
				bmp[j].writeData("shrink", j);
				bmp[j].restore();
			}
		}
		break;
	}

	return 0;
}