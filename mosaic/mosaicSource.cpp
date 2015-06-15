#include<opencv2\core.hpp>
#include<opencv2\highgui.hpp>
#include<opencv2\imgproc.hpp>
#include<stdio.h>

using namespace cv;
using namespace std;

void readme()
{
	printf("mosaic requires at least one input parameter, the path to the image\n");
	printf("you may also include an output filename template WITHOUT the file extension\n");
}

void blockSz(int blockSize, void *);
void mosaicIm(int blockSize, Mat im, Mat imMosaic, string mosaicName);
Mat im2colstep(Mat im, Size patchSize, Size stepSize);
Mat col2imstep(Mat in, Size imSize, Size patchSize, Size stepSize);

int main(int argc, char ** argv)
{

	if (argc == 1)
	{
		readme();
		return 0;
	}

	//string imFN = "lena.jpg";
	string imFN = argv[1];
	string outFN;
	if (argc == 3)
		outFN = argv[2];
	else
		outFN = "output_";

	// read in the image
	Mat im = imread(imFN.c_str(), IMREAD_ANYCOLOR);
	if (!im.data)
	{
		printf("Couldn't read %s. Quitting\n", imFN.c_str());
		return -1;
	}

	// allocate the output image
	Mat imMosaic(im.size(), CV_8UC(im.channels()));

	// display the input image
	string winName = "Input Image";
	namedWindow(winName, WINDOW_NORMAL);
	resizeWindow(winName, im.cols, im.rows);
	moveWindow(winName, 10, 10);

	// create a trackbar
	string trackName = "Block Size";
	int blockSize = 1;
	createTrackbar(trackName, winName, &blockSize, 32, blockSz);

	// make the output window
	string mosaicName = "Output Mosaic";
	namedWindow(mosaicName, WINDOW_NORMAL);
	resizeWindow(mosaicName, im.cols, im.rows);
	moveWindow(mosaicName, 10 + im.cols + 10, 10);

	char key = 'a';
	int prevSz = blockSize;
	do{
		imshow(winName, im);
		key = waitKey(100);
		if (prevSz != blockSize)
		{
			prevSz = blockSize;
			mosaicIm(blockSize, im, imMosaic, mosaicName);
		}

		//if (key == ' ')
		//{
		//	mosaicIm(blockSize, im, imMosaic, mosaicName);
		//	key = 'a';
		//}
		if (key == 's')
		{
			char saveFN[200];
			sprintf(saveFN, "%s%02d.png", outFN.c_str(), blockSize);
			imwrite(saveFN, imMosaic);
			key = 'a';
		}
	} while (key != 'q');

	return 0;
}

void blockSz(int blockSize, void *)
{
	if (blockSize == 0)
		blockSize = 1;

	return;
}

void mosaicIm(int blockSize, Mat im, Mat imMosaic, string mosaicName)
{
	if (blockSize == 1)
	{
		imMosaic = im.clone();
		imshow(mosaicName, imMosaic);
		waitKey(100);
		return;
	}

	int nChans = im.channels();
	// split the image into channels
	vector<Mat> chans(nChans);

	Size patchSize(blockSize, blockSize);
	Size patchShift(blockSize, blockSize);

	for (int i = 0; i < nChans; i++)
	{ 
		extractChannel(im, chans[i], i);
		Mat tmp = im2colstep(chans[i], patchSize,patchShift);
		// find the average along the rows
		reduce(tmp, tmp, 1, REDUCE_AVG);
		// pad back to the correct size
		copyMakeBorder(tmp, tmp, 0, 0, 0, patchSize.area()-1, BORDER_REPLICATE);
		// reshape back to the image size
		chans[i] = col2imstep(tmp, im.size(), patchSize, patchShift);
	}


	// merge the channels back into an image
	merge(chans, imMosaic);

	imshow(mosaicName, imMosaic);
	waitKey(100);

	return;
}

/*
im2colstep and col2imstep have been adapted from mex files written by Ron Rubinstein at Technion for his KSVD-Box software
*/

Mat im2colstep(Mat im, Size patchSize, Size stepSize)
{
	int totalPatches = ((im.cols - patchSize.width) / stepSize.width + 1)*((im.rows - patchSize.height) / stepSize.height + 1);
	int patchLen = patchSize.area()*im.channels();

	Mat out(totalPatches, patchLen, CV_8UC1); 
	int p0 = im.channels(), p1 = patchSize.width, p2 = patchSize.height;
	int s0 = im.channels(), s1 = stepSize.width, s2 = stepSize.height;
	int i0 = im.channels(), i1 = im.cols, i2 = im.rows;
	unsigned char * imData = (unsigned char *)im.data;
	unsigned char * outData = (unsigned char *)out.data;

	// extract patches
	int blocknum = 0;
	for (int k = 0; k <= i2 - p2; k += s2)
	{
		for (int j = 0; j <= i1 - p1; j += s1)
		{
			for (int i = 0; i <= i0 - p0; i += s0)
			{
				// copy a block
				for (int m = 0; m < p2; m++)
				{
					for (int n = 0; n < p1; n++)
					{
						memcpy(outData + blocknum*p0*p1*p2 + m*p0*p1 + n*p0, imData + (k + m)*i0*i1 + (j + n)*i0 + i, i0*sizeof(unsigned char));
					}
				}

				blocknum++;
			}
		}
	}

	return out;
}

Mat col2imstep(Mat in, Size imSize, Size patchSize, Size stepSize)
{
	int chan = in.cols / patchSize.area();
	int i0 = chan, i1 = imSize.width, i2 = imSize.height;
	int p0 = chan, p1 = patchSize.width, p2 = patchSize.height;
	int s0 = chan, s1 = stepSize.width, s2 = stepSize.height;

	// Output matrix
	Mat out = Mat::zeros(imSize, CV_8UC(chan));

	unsigned char * iData = (unsigned char *)in.data;
	unsigned char * oData = (unsigned char *)out.data;

	int blocknum = 0;

	for (int k = 0; k <= i2 - p2; k += s2)
	{
		for (int j = 0; j <= i1 - p1; j += s1)
		{
			for (int i = 0; i <= i0 - p0; i += s0)
			{
				// add back a single patch
				for (int m = 0; m < p2; m++)
				{
					for (int n = 0; n < p1; n++)
					{
						for (int t = 0; t < p0; t++)
						{
							(oData + (k + m)*i0*i1 + (j + n)*i0 + i)[t] += (iData + blocknum*p0*p1*p2 + m*p0*p1 + n*p0)[t];
						}
					}
				}
				blocknum++;
			}
		}
	}

	return out;
}