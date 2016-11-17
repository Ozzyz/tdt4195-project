import numpy as np
import cv2
from matplotlib import pyplot as plt

def region_grow(img, points, t):
    """Region growing algorithm with 4-connectedness

    img:    grayscale image
    points: list of (x,y) tuples
    t:      threshold
    return a list of new points
    """
    w, h = img.shape
    Q = list(points) # initialize queue
    result = list(points)
    while Q:
        poi = Q.pop() # point of interest
        x, y = poi
        left = (x - 1, y)
        if img[y, x - 1] > t:
            if left not in result:
                Q.append(left)
                result.append(left)
        right = (x + 1, y)
        if img[y, x + 1] > t:
            if right not in result:
                Q.append(right)
                result.append(right)
        down = (x, y + 1)
        if img[y + 1, x] > t:
            if down not in result:
                Q.append(down)
                result.append(down)
        up = (x, y - 1)
        if img[y - 1, x] > t:
            if up not in result:
                Q.append(up)
                result.append(up)
    return result

def make_image(points, old_img):
    """Create a binary image from a list of points with size of another image
    """
    new_img = np.zeros_like(old_img)
    for point in points:
        x, y = point
        new_img[y, x] = 1
    return new_img

def blur_threshold_close(img):
	gray = cv2.cvtColor(img,cv2.COLOR_RGB2GRAY)
	gray = 255 - gray

	blur = cv2.medianBlur(gray, 11)

	#ret, thresh = cv2.threshold(blur,127,255,cv2.THRESH_BINARY+cv2.THRESH_OTSU)
	th2 = 255-cv2.adaptiveThreshold(blur,255,cv2.ADAPTIVE_THRESH_MEAN_C,\
     		cv2.THRESH_BINARY, 11,2)
	#new = cv2.erode(th2, np.ones((3,3),np.uint8), iterations=1)
	new = cv2.morphologyEx(th2, cv2.MORPH_CLOSE, np.ones((3,3),np.uint8), iterations=2)	
	return new
	
def main():
	img = cv2.imread('images/easy01.png')

	new = blur_threshold_close(img)
	
	plt.figure()
	plt.imshow(new, cmap=plt.cm.gray)
	plt.show()


if __name__ == "__main__":
	main()
