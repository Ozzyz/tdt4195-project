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
    gray = cv2.cvtColor(img,cv2.COLOR_RGBA2GRAY)
    #blur = cv2.medianBlur(gray, 7)
    blur = cv2.bilateralFilter(gray, 7, 75, 75)
    blur = 255 - blur

    #ret, th2 = cv2.threshold(blur,127,255,cv2.THRESH_BINARY+cv2.THRESH_OTSU)
    th = cv2.adaptiveThreshold(blur, 255,cv2.ADAPTIVE_THRESH_MEAN_C,\
            cv2.THRESH_BINARY_INV, 13,2)
    kernel = np.ones((3,3), np.uint8)
    new = cv2.dilate(th, np.ones((3,3), np.uint8), iterations=0)
    new = cv2.erode(new, np.ones((3,3), np.uint8), iterations=0)
    new = cv2.dilate(new, np.ones((3,3), np.uint8), iterations=0)
    new = cv2.erode(new, np.ones((3,3), np.uint8), iterations=0)

    new = cv2.morphologyEx(new, cv2.MORPH_CLOSE, np.ones((3,3),np.uint8),
            iterations=0)	
    
    return new

def canny_test(img):
    denoised = cv2.fastNlMeansDenoisingColored(img, None, 30, 10, 7, 21)
    #filtered = cv2.bilateralFilter(denoised, 9, 75, 75) 
    gray = cv2.cvtColor(denoised, cv2.COLOR_RGB2GRAY)
    th = cv2.adaptiveThreshold(gray, 255, cv2.ADAPTIVE_THRESH_MEAN_C,\
            cv2.THRESH_BINARY_INV, 11, 2)
    th2 = cv2.morphologyEx(th, cv2.MORPH_CLOSE, np.ones((3,3),np.uint8),
            iterations=1)	
    result = cv2.Canny(th, 100, 255)
    return th


def testing():
    img = cv2.imread('images/easy01.png')
    img_copy = img.copy()

    # canny
    edges = cv2.Canny(cv2.cvtColor(img, cv2.COLOR_RGB2GRAY), 100, 255)
    edges_copy = edges.copy()

    th = blur_threshold_close(img)
    th_copy = th.copy()

    im2, c, h = cv2.findContours(th_copy, 1, 2)
    #cv2.drawContours(img_copy, c, -1, (0,255,0), 3)

    real_contours = [cnt for cnt in c if 100 < cv2.contourArea(cnt) < 8000]
    cv2.drawContours(img_copy, real_contours, -1, (0, 255, 0), 3)

    for c in real_contours:
        print(cv2.contourArea(c))
    
    # more testing
    can = canny_test(img)

    out = img
    plt.figure()
    plt.imshow(out, cmap=plt.cm.gray)
    plt.show()

def more_testing():
    im_in = cv2.imread("images/easy01.png");
    gray = cv2.cvtColor(im_in, cv2.COLOR_RGB2GRAY)
    gray = cv2.medianBlur(gray, 5)
    gray = 255 - gray
    # Threshold.
    # Set values equal to or above 220 to 0.
    # Set values below 220 to 255.
    #th, im_th = cv2.threshold(gray, 20, 255,
    #        cv2.THRESH_BINARY+cv2.THRESH_OTSU);
    im_th = cv2.adaptiveThreshold(gray,255,cv2.ADAPTIVE_THRESH_MEAN_C,\
                cv2.THRESH_BINARY_INV, 11,2)
    
    # Copy the thresholded image.
    im_floodfill = im_th.copy()
     
    # Mask used to flood filling.
    # Notice the size needs to be 2 pixels than the image.
    h, w = im_th.shape[:2]
    mask = np.zeros((h+2, w+2), np.uint8)
     
    # Floodfill from point list
    #points = [(355, 43), (434, 22), (759, 43), (144, 144), ]
    
    points = [(355,150)]
    for point in points:
        cv2.floodFill(im_floodfill, mask, point, 255);
     
    # Invert floodfilled image
    im_floodfill_inv = cv2.bitwise_not(im_floodfill)
     
    # Combine the two images to get the foreground.
    im_out = im_th | im_floodfill_inv
    
    #plt.figure()
    #plt.imshow(im_out, cmap=plt.cm.gray)
    #plt.show()

    # Display images.
    #cv2.imshow("Grayscale image", gray)
    #cv2.imshow("Blurred", blur)
    cv2.imshow("Thresholded Image", im_th)
    cv2.imshow("Floodfilled Image", im_floodfill)
    cv2.imshow("Inverted Floodfilled Image", im_floodfill_inv)
    cv2.imshow("Foreground", im_out)
    cv2.waitKey(0)

from queue import Queue as Q
def region_grow(seeds, dT, img):
    segmented = np.empty((img.shape[0], img.shape[1]))
    visited = set()
    for seed in seeds:
        queue = Q()
        queue.put(seed)
        x0, y0 = seed  # coordinates of seed
        seed_int = img[y0,x0]  # intensity value of seed
        while not queue.empty():
            print(len(visited))
            pixel = queue.get()
            for neighbour in get_neighbours(pixel, img): # returns a set of the pixels Moore neighbourhood
                i, j = neighbour
                if neighbour not in visited:
                    visited.add(neighbour)
                    queue.put(neighbour)
                    for c in range(3):
                        diff = np.abs(int(seed_int[c] - img[i][j][c]))
                        if diff in range(-dT, dT):  # we expand on every pixel within our intensity range
                            segmented[i][j] = 255  # pixels within the range are set to 1

    return segmented

def get_neighbours(pixel, img):
    neighbours = set()
    x, y = pixel

    for i in range(x - 1, x + 2):
        for j in range(y - 1, y + 2):
            if (i == x and j == y) or i not in range(img.shape[0]) or j not in range(img.shape[1]):
                continue  # avoid pixels out of bound as well as source pixel
            else:
                neighbours.add((i, j))

    return neighbours

def grow_testing():
    img = cv2.imread("images/easy01.png")
    #points = [(355,150), (555, 44), (355, 254), (656, 248), (147, 250), (144,
    #    450), (460, 455), (770, 450), (757, 50)]
    points = [(355, 150)]
    out = region_grow(points, 2, img)
    cv2.imshow("asd", out)



def main():
    #grow_testing()
    #more_testing()
    testing()
    #img = cv2.imread("images/easy01.png")
    #lab = cv2.cvtColor(img, cv2.COLOR_RGB2Lab)
    #cv2.imshow("lab", lab)
    #cv2.waitKey()



if __name__ == "__main__":
	main()
