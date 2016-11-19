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

def filter_contours(contours, low=100, high=10000):
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
    epsilon = 0.04 * cv2.arcLength(cnt, True)
    approx = cv2.approxPolyDP(cnt, epsilon, True) 
    ratio = cv2.contourArea(cnt) / cv2.arcLength(cnt, True)
    return len(approx), ratio

def match_shape(contour):
    vertices, rat = ratio(contour)
    if vertices == 3:
        shape = "Triangle"
    elif vertices == 4:
        if rat < 6:
            shape = "Arrow"
        else:
            shape = "Paralellogram"
    elif vertices == 6:
        if rat > 12:
            shape = "Hexagon"
        else:
            shape = "Star"
    else:
        shape = "Pacman"
    return shape, vertices, rat


def read_seeds(filepath):
    """Return a list of tuples containing seed points, read from a file
    """
    a =  [line.strip().replace(",","").split() for line in open(filepath).readlines()]
    return [(int(b[0]), int(b[1])) for b in a]

if __name__ == "__main__":
    import sys
    if len(sys.argv) == 4:
        img = cv2.imread(sys.argv[1])
        seeds = read_seeds(sys.argv[2])
    else:
        print("Usage: contours.py <imagepath> <seedspath>")
        sys.exit()
    
    a = 0
    b = 100

    img = cv2.GaussianBlur(img, (5,5), 0)
    # Region grow
    grown = region_grow(img, seeds, int(sys.argv[3]))
    plt.figure()
    plt.imshow(grown, cmap=plt.cm.gray)
    plt.show()
    # Find all contours
    contours = filter_contours(find_contours(grown))
    
    # Draw contours
    contoured_img = draw_contours(img, [cnt for cnt in contours if 
       a < ratio(cnt)[0] < b ])
    
    # Find centroids 
    centroids = find_centroids(contours)
    
    # Draw centroids
    img_with_centroids = draw_centroids(img, centroids)

    # Match shapes
    matches = []
    for cnt in contours:
        c, wh, a = cv2.minAreaRect(cnt)
        bnd_rect_diff = wh[0]*wh[1] - cv2.contourArea(cnt) 
        print("bnd_rect_diff: {}\tarclength: {}\tcontourArea:\
            {}".format(int(bnd_rect_diff), int(cv2.arcLength(cnt, True)),
                int(cv2.contourArea(cnt))))
        matches.append(match_shape(cnt))
    import pprint
    pprint.PrettyPrinter().pprint(sorted(matches, key=lambda x: x[0]))
   
    # Show image
    cv2.imshow("asd", contoured_img)
    cv2.waitKey()

    

