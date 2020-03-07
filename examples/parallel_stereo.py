import cv2
import numpy as np
import math

# Load Data
print("Loading data...")
imgL = cv2.cvtColor(cv2.imread('C:\\Users\\yafai\\Desktop\\DisSys\\CSCE689-DRS\\GroundTest\\12_Dec_19\\approach1\\EO\\3L_eo.bmp'), cv2.COLOR_RGB2GRAY)
imgR = cv2.cvtColor(cv2.imread('C:\\Users\\yafai\\Desktop\\DisSys\\CSCE689-DRS\\GroundTest\\12_Dec_19\\approach1\\EO\\3R_eo.bmp'), cv2.COLOR_RGB2GRAY)
mtxL = np.loadtxt(open("C:\\Users\\yafai\\Desktop\\DisSys\\CSCE689-DRS\\examples\\perfect_calibrations_4K\\M1.dat", "rb"), delimiter=" ", skiprows=0, usecols=(0,1,2)).astype("float")
mtxR = np.loadtxt(open("C:\\Users\\yafai\\Desktop\\DisSys\\CSCE689-DRS\\examples\\perfect_calibrations_4K\\M2.dat", "rb"), delimiter=" ", skiprows=0, usecols=(0,1,2)).astype("float")
distL = np.loadtxt(open("C:\\Users\\yafai\\Desktop\\DisSys\\CSCE689-DRS\\examples\\perfect_calibrations_4K\\d1.dat", "rb"), delimiter=" ", skiprows=0, usecols=(0,1,2,3,4)).astype("float")
distR = np.loadtxt(open("C:\\Users\\yafai\\Desktop\\DisSys\\CSCE689-DRS\\examples\\perfect_calibrations_4K\\d2.dat", "rb"), delimiter=" ", skiprows=0, usecols=(0,1,2,3,4)).astype("float")
R = np.loadtxt(open("C:\\Users\\yafai\\Desktop\\DisSys\\CSCE689-DRS\\examples\\perfect_calibrations_4K\\R.dat", "rb"), delimiter=" ", skiprows=0, usecols=(0,1,2)).astype("float")
T = np.loadtxt(open("C:\\Users\\yafai\\Desktop\\DisSys\\CSCE689-DRS\\examples\\perfect_calibrations_4K\\T.dat", "rb"), delimiter=" ", skiprows=0, usecols=(0,1,2)).astype("float")

# One time operations to fix the stereo images
print("Initializing StereoBM...")
imageSize = imgL.shape[::-1]
R1, R2, P1, P2, Q, temp1, temp1 =  cv2.stereoRectify(mtxL, distL, mtxR, distR, imageSize, R, T)
mapLx, mapLy = cv2.initUndistortRectifyMap(mtxL, distL, R1, P1, imageSize, cv2.CV_16SC2)
mapRx, mapRy = cv2.initUndistortRectifyMap(mtxR, distR, R2, P2, imageSize, cv2.CV_16SC2)
numDisparities=256
blockSize=21
stereo = cv2.StereoBM_create(numDisparities, blockSize)

# Fix the individual image single-threaded
print("Fixing input images...")
remapL = cv2.remap(imgL, mapLx, mapLy, cv2.INTER_LINEAR)
remapR = cv2.remap(imgR, mapRx, mapRy, cv2.INTER_LINEAR)

# Testing the algorithm!
print("Running StereoBM")
disparity = stereo.compute(remapL,remapR)
disparity = cv2.normalize(disparity, None, alpha=0, beta=1, norm_type=cv2.NORM_MINMAX, dtype=cv2.CV_32F)
cv2.imshow('Disparity', disparity)

print("Segmenting Image")
segments = 4
segmentedSize = (imageSize[0] / segments, imageSize[1] / segments)
disparityTest = np.zeros((imageSize[1], imageSize[0]), np.int16)

print("Image Size:     " + str(imageSize))
print("Segmented Size: " + str(segmentedSize))
print("Disparity Size: " + str(disparityTest.shape))

for yInd in range(0, segments):
	for xInd in range(0, segments):
		print(str(xInd) + ", " + str(yInd))
		xStart = int(max(0, xInd * segmentedSize[0] - numDisparities - blockSize/2-1))
		yStart = int(max(0, yInd * segmentedSize[1] - blockSize/2-1))
		xEnd = int(min(imageSize[0], (xInd+1) * segmentedSize[0] + blockSize/2+1))
		yEnd = int(min(imageSize[1], (yInd+1) * segmentedSize[1] + blockSize/2+1))
		
		xStartTransfer = int(xInd * segmentedSize[0])
		xEndTransfer = int(xStartTransfer + segmentedSize[0])
		yStartTransfer = int(yInd * segmentedSize[1])
		yEndTransfer = int(yStartTransfer + segmentedSize[1])
		
		xStartOffset = xStartTransfer - xStart
		xEndOffset = xEnd - xEndTransfer
		yStartOffset = yStartTransfer - yStart
		yEndOffset = yEnd - yEndTransfer
		
		print("    " + str(xStart) + ", " + str(yStart) + "   " + str(xEnd) + ", " + str(yEnd))
		print("    " + str(xStartTransfer) + ", " + str(yStartTransfer) + "   " + str(xEndTransfer) + ", " + str(yEndTransfer))
		print("    " + str(xStartOffset) + ", " + str(yStartOffset) + "   " + str(xEndOffset) + ", " + str(yEndOffset))
		#print("    " + str(int(len(segment[1])-xStartOffset-xEndOffset)) + ", " + str(int(len(segment[0])-yStartOffset-yEndOffset)))
		
		segment = stereo.compute(remapL[yStart:yEnd, xStart:xEnd],remapR[yStart:yEnd, xStart:xEnd])
		print("    " + str(segment.shape))
		disparityTest[yStartTransfer:yEndTransfer,xStartTransfer:xEndTransfer] = segment[yStartOffset:int(segment.shape[0]-yEndOffset), xStartOffset:int(segment.shape[1]-xEndOffset)]
		
disparityTest = cv2.normalize(disparityTest, None, alpha=0, beta=1, norm_type=cv2.NORM_MINMAX, dtype=cv2.CV_32F)
for xInd in range(0, segments):
	cv2.line(disparityTest, (0, int(segmentedSize[1]) * xInd), (imageSize[0], int(segmentedSize[1]) * xInd), 255, 1)
for yInd in range(0, segments):
	cv2.line(disparityTest, (int(segmentedSize[0]) * yInd, 0), (int(segmentedSize[0]) * yInd, imageSize[1]), 255, 1)

cv2.imshow('Disparity Test', disparityTest)
print(np.array_equal(disparity, disparityTest))
cv2.imshow('Left', remapL)
cv2.imshow('Right', remapR)
cv2.waitKey()
cv2.destroyAllWindows()

