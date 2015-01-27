#include "stdafx.h"
#include "VIBE.h"

void initRnd(unsigned int size)
{
	unsigned int seed = time(0);

	srand(seed);
	rndSize=size;
	rnd=(unsigned int*)calloc(rndSize, sizeof(unsigned int));

	for(unsigned int i=0; i < rndSize; i++)
	{
		rnd[i] = rand();
	}

	rndPos=0;
}

unsigned int freeRnd()
{
	free(rnd);
	rndSize=0;
	return 0;
}

vibeModel *libvibeModelNew()
{
	vibeModel *model = (vibeModel*)calloc(1, sizeof(vibeModel));

	if(model)
	{
		model->numberOfSamples = 20;
		model->matchingThreshold = 20;
		model->matchingNumber = 2;
		model->updateFactor = 16;
		initRnd(65536);
	}

	return model;
}

unsigned char getRandPixel(const unsigned char *image_data, const unsigned int width, const unsigned int height, const unsigned int stride, const unsigned int x, const unsigned int y)
{
	unsigned int neighborRange=1;
	int dx;
	int dy;

	dx = (x-neighborRange) + rnd[rndPos=(rndPos+1)%rndSize]%(2*neighborRange);
	dy = (y-neighborRange) + rnd[rndPos=(rndPos+1)%rndSize]%(2*neighborRange);

	if(( dx < 0) || ( dx>=width ))
	{
		if(dx<0)
		{
			dx = rnd[rndPos=(rndPos+1)%rndSize]%(x+neighborRange);
		}
		else
		{
			dx = (x-neighborRange) + rnd[rndPos=(rndPos+1)%rndSize]%(width - x + neighborRange-1);
		}
	}


	if((dy<0)||(dy>=height))
	{
		if(dy<0)
		{
			dy = rnd[rndPos=(rndPos+1)%rndSize]%(y+neighborRange);
		}
		else
		{
			dy = (y-neighborRange) + rnd[rndPos=(rndPos+1)%rndSize]%(height - y + neighborRange-1);
		}
	}

	return image_data[dx+dy*stride];
}

int libvibeModelInit(vibeModel *model, const unsigned char *image_data, const unsigned int width, const unsigned int height, const unsigned int stride)
{
	if (!model || !image_data || !width || !height || !stride || (stride < width)) 
		return 1;

	// Lưu kích thước ảnh
	model->width  = width;
	model->height = height;
	model->stride = stride;

	// Cấp bộ nhớ cho mỗi model pixel
	model->pixels = 0;
	model->pixels = (pixel*)calloc(model->width*model->height, sizeof(pixel));
	if (!model->pixels) 
		return 1;

	// Cấp bộ nhớ
	for (unsigned int i=0; i < model->width*model->height; i++)
	{
		model->pixels[i].numberOfSamples = model->numberOfSamples;
		model->pixels[i].sizeOfSample = 1;
		model->pixels[i].samples = 0;
		model->pixels[i].samples = (unsigned char*)calloc(model->numberOfSamples,sizeof(unsigned char));
		if (!model->pixels[i].samples) 
			return 1;
	}

	// Cấp dữ liệu cho model
	unsigned int n=0;
	for (unsigned int j=0; j < model->height; j++)
	{
		for (unsigned int i=0; i < model->width; i++)
		{
			model->pixels[n].samples[0] = image_data[i + j*stride];

			for (unsigned int k=1; k < model->numberOfSamples; k++)
				model->pixels[n].samples[k] = getRandPixel(image_data, width, height, stride, i, j);
			n++;
		}
	}

	return 0;
}


int libvibeModelUpdate(vibeModel *model, const unsigned char *image_data, unsigned char *segmentation_map)
{
	int ad = model->stride - model->width;

	if (!model || !image_data || !segmentation_map) 
		return 1;

	if (model->stride < model->width) 
		return 1;

	unsigned int n=0;
	for (int j=0; j < model->height; j++)
	{
		for (int i=0; i < model->width; i++)
		{

			// So sánh pixel
			bool flag=false;
			unsigned int matchingCounter=0;

			// So sánh với tất cả các mẫu
			for(unsigned int t=0; t < model->pixels[n].numberOfSamples; t++)
			{               
				//Nếu sự khác nhau nhỏ hơn ngưỡng
				if (abs((int)image_data[n]-(int)model->pixels[n].samples[t]) < model->matchingThreshold) 
				{
					matchingCounter++;
					if (matchingCounter >= model->matchingNumber)
					{
						flag=true;
						break;
					}
				}
			}
			
			if(flag)
			{
				// matches background - update modet for this point
				segmentation_map[n] = 0;

				/****************************************************************/
				/**********************Model update**************************/
				/****************************************************************/
				if(rnd[rndPos=(rndPos+1)%rndSize]%model->updateFactor)
				{
					model->pixels[i+model->width*j].samples[rnd[rndPos=(rndPos+1)%rndSize]%model->numberOfSamples]=image_data[n];

					unsigned int m = (model->stride * j + i);
					switch((rnd[rndPos=(rndPos+1)%rndSize])%8) 
					{
					case 0:
						if ((model->width - 1) <= i) 
						{
						}
						else 
						{
							m++;
						}
						break;
					case 1: 
						if ((model->width - 1) <= i) 
						{
						}
						else
						{
							m++;
						} 
						if ((model->height - 1) <= j) 
						{
						}
						else
						{
							m += model->stride;
						}
						break;
					case 2: 
						if ((model->height - 1) <= j) 
						{
						}
						else 
						{
							m += model->stride;
						}
						break;
					case 3: 
						if (i <= 0) 
						{
						}
						else 
						{
							m--;
						}
						if ((model->height - 1) <= j) 
						{
						}
						else 
						{
							m += model->stride;
						}
						break;
					case 4: 
						if (i <= 0) 
						{
						}
						else 
						{
							m--;
						}
						break;
					case 5: 
						if (i <= 0) 
						{
						}
						else 
						{
							m--;
						}
						if (j <= 0) 
						{
						}
						else 
						{
							m -= model->stride;
						}
						break;
					case 6: 
						if (j <= 0) 
						{
						}
						else
						{
							m -= model->stride;
						}
						break;
					case 7: 
						if ((model->width - 1) <= i) 
						{
						}
						else
						{
							m++;
						}
						if (j <= 0) 
						{
						} 
						else 
						{
							m -= model->stride;
						}
						break;
					default:
						puts("You should not see this message!!!");
						break;
					}
					model->pixels[m].samples[rnd[rndPos=(rndPos+1)%rndSize]%model->numberOfSamples]=image_data[n];
					/****************************************************************/
					/****************************************************************/
					/****************************************************************/
				}
			}
			else
			{
				// background difference
				segmentation_map[n] = 255;
			}
			n++;
		}
		if (model->stride > model->width)
			n+=ad;
	}
	return 0;
}

int libvibeModelFree(vibeModel *model)
{
	for(unsigned int i=0; i<model->width*model->height; i++)
	{
		free(model->pixels[i].samples);
	}
	free(model->pixels);
	freeRnd();
	return 0;
}
