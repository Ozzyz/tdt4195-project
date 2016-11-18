import numpy as np
import cv2
from matplotlib import pyplot as plt
from region_growing import *

def find_contours(img):
    """Return contours of a grayscale image
    """
    img_copy = img.copy()
    im2, contours, h = cv2.findContours(img_copy, 1, 2)
    return contours

def draw_contours(img, contours):
    """Return copy of input image with contours drawn
    """
    img_copy = img.copy()
    cv2.drawContours(img_copy, contours, -1, (0, 255, 0), 3)
    return img_copy


def match_shape(shapes, contour):
    # In: A dictionary of shapes that will be matched
    # with a given contour. The output will be the name of the shape
    match_vals = []
    for shape in shapes:
        res = cv2.matchShapes(contour, shapes[shape], 1, 0, 0)
        match_vals.append((res, shape))
    # Compare matching score of shapes, which is first in the list
    best_matching_shape = min(match_vals, key= lambda x: x[0])
    return best_matching_shape[0]

def create_shape_contours():
    fp = "images/"
    names = ["star", "trapezoid_horisontal", "trapezoid_vertical", "pacman", "hexagon", "arrow", "triangle"]
    f_extension = ".png"
    contour_dict = {}
    for name in names:
        img = cv2.imread(fp+name+f_extension)
        contours[name] = img
    return contour_dict
    

def find_centroids(contours):
    """Return list of centroids from contours
    """
    img_copy = img.copy()
    centroids = []
    for cnt in contours:
        M = cv2.moments(cnt)
        cx = int(M["m10"]/M["m00"])
        cy = int(M["m01"]/M["m00"])
        centroids.append((cx, cy))
    return centroids

def draw_centroids(img, centroids, radius=5, color=(255,0,0)):
    """Return a new image with circles for every centroid.
    """
    img_copy = img.copy()
    for c in centroids:
        cv2.circle(img_copy, c, radius, color, -1)
    return img_copy

def filter_contours(contours, low=50, high=10000):
    """Return a new list of contours, filtered by area between low and high
    """
    return [cnt for cnt in contours \
            if low < cv2.contourArea(cnt) < high]
>>>>>>> fcfaba3504d52577925d928e76518249424a7ac6

if __name__ == "__main__":
    img = cv2.imread("images/easy01.png")
    seeds = [(149, 449), (141, 247), (355,150), (355, 254), (355, 43), (434, 22), (760, 39), (746, 70), (742, 26), (759, 438), (145, 148), (549, 153), (660, 252), (251, 346), (357, 349), (652, 355), (52, 451), (455, 447)]
    grown = region_grow(img, seeds, 40)
    c = filter_contours(find_contours(grown), 50, 10000)
    c_img = draw_contours(img, c)
    centroids = find_centroids(c)
    img_with_centroids = draw_centroids(img, centroids)

    cv2.imshow("asd", img_with_centroids)
    cv2.waitKey()

    

