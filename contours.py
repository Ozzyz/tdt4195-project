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
    y,x,w,h = cv2.boundingRect(cnt)
    return (w*h - cv2.contourArea(cnt)) / cv2.arcLength(cnt, True)
    
def match_shape(contour):
    shapes = {
        "arrow": (0.0, 6.0),
        "star": (8.5, 9.5),
        "triangle": (10.0, 10.15),
        "pacman": (10.2, 10.6),
        "trapezoid": (13.0, 14.0),
        "hexagon": (14.9, 15.6)
    }

    for name, value in shapes.items():
        if value[0] < ratio(contour) < value[1]:
            return name, ratio(contour)
    return "Unknown shape", ratio(contour)

def read_seeds(filepath):
    """Return a list of tuples containing seed points, read from a file
    """
    a =  [line.strip().replace(",","").split() for line in open(filepath).readlines()]
    return [(int(b[0]), int(b[1])) for b in a]

if __name__ == "__main__":
    import sys
    if len(sys.argv) == 3:
        img = cv2.imread(sys.argv[1])
        seeds = read_seeds(sys.argv[2])
    else:
        print("Usage: contours.py <imagepath> <seedspath>")
        sys.exit()

    # Region grow
    grown = region_grow(img, seeds, 40)
    
    # Find all contours
    contours = filter_contours(find_contours(grown))
    
    # Draw contours
    contoured_img = draw_contours(img, [cnt for cnt in contours])
    
    # Find centroids 
    centroids = find_centroids(contours)
    
    # Draw centroids
    img_with_centroids = draw_centroids(img, centroids)

    # Match shapes
    matches = []
    for cnt in contours:
        matches.append(match_shape(cnt))
    import pprint
    pprint.PrettyPrinter().pprint(sorted(matches, key=lambda x: x[1]))
   
    # Show image
    cv2.imshow("asd", contoured_img)
    cv2.waitKey()

    

