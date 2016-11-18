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

def show_contoured_image(img):
    gray = cv2.cvtColor(img, cv2.COLOR_RGB2GRAY)
    c = find_contours(gray)
    c_img = draw_contours(img, c)
    cv2.imshow("asd", c_img)
    cv2.waitKey()

def ratio(cnt):
    return cv2.contourArea(cnt) / cv2.arcLength(cnt, True)

def match_shape(contour):
    shapes = {
        "arrow": (0.0, 5.0),
        "star": (6.0, 9.0),
        "triangle": (10.0, 10.15),
        "pacman": (10.2, 10.3),
        "trapezoid": (13.0, 14.0),
        "hexagon": (14.9, 15.6)
    }
    for name, value in shapes.items():
        if value[0] < ratio(contour) < value[1]:
            return name
    return "Unknown shape"

def read_seeds(filepath):
    a =  [line.strip().replace(",","").split() for line in open("seeds_easy01.txt").readlines()]
    print(a)
    return [(int(b[0]), int(b[1])) for b in a]

if __name__ == "__main__":
    img = cv2.imread("images/easy01.png")
    seeds = read_seeds("seeds_easy01.txt")
    print(seeds)
    grown = region_grow(img, seeds, 40)
    c = filter_contours(find_contours(grown))
    c_img = draw_contours(img, c)
    centroids = find_centroids(c)
    img_with_centroids = draw_centroids(img, centroids)

    for cnt in c:
        print(match_shape(cnt))

    cv2.imshow("asd", c_img)
    cv2.waitKey()

    

