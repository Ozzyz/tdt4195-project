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

def find_centroid(contour):
    """Return centroid from a contour
    """
    M = cv2.moments(contour)
    cx = int(M["m10"]/M["m00"])
    cy = int(M["m01"]/M["m00"])
    return (cx, cy)

def draw_centroids(img, centroids, radius=5, color=(255,0,0)):
    """Return a new image with circles for every centroid.
    """
    img_copy = img.copy()
    for c in centroids:
        cv2.circle(img_copy, c, radius, color, -1)
    return img_copy

def filter_contours(contours, low=400, high=10000):
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
    # Estimate accuracy of perimeter to 4% 
    epsilon = 0.04 * cv2.arcLength(cnt, True)
    # Approximate contour with polygon 
    approx = cv2.approxPolyDP(cnt, epsilon, True)
    vertices = len(approx)
    ratio = cv2.contourArea(cnt) / cv2.arcLength(cnt, True)
    return vertices, ratio

def match_shape(contour):
    """Return name of a shape based on its contours
    """
    vertices, area_perim_ratio = ratio(contour)
    if vertices == 3:
        shape = "Triangle"
    elif vertices == 4:
        if area_perim_ratio < 6:
            shape = "Arrow"
        else:
            shape = "Paralellogram"
    elif vertices == 6:
        if area_perim_ratio > 12:
            shape = "Hexagon"
        else:
            shape = "Pacman"
    else:
        shape = "Star"
    return shape


def read_seeds(filepath):
    """Return a list of tuples containing seed points, read from a file
    """
    a =  [line.strip().replace(",","").split() for line in open(filepath).readlines()]
    return [(int(b[0]), int(b[1])) for b in a]

def plot(img, title, cm=plt.cm.gray):
    plt.figure()
    plt.imshow(img, cmap=cm)
    plt.axis("off")
    plt.title(title)
    plt.show()

def write_to_file(shapes_and_centroids):
   
    # Write output files
    fn = sys.argv[1].split("/")
    with open("shapes_and_centroids/" + fn[1] + ".txt", "w") as f:
        for item in shapes_and_centroids:
            shape, centroid = item
            x, y = centroid
            f.write(shape + " " + str(x) + " " + str(y) + "\n")



if __name__ == "__main__":
    import sys
    if len(sys.argv) == 4:
        img = cv2.imread(sys.argv[1])
        seeds = read_seeds(sys.argv[2])
        threshold = int(sys.argv[3])
    else:
        print("Usage: contours.py <imagepath> <seedspath> <threshold>")
        sys.exit()
    
    # Blur
    img = cv2.GaussianBlur(img, (5,5), 0)

    # Region grow
    grown = region_grow(img, seeds, threshold)
    
    # Find all contours
    contours = find_contours(grown)
    
    # Filter contours
    filtered_contours = filter_contours(contours)
    
    # Draw contours
    contoured_img = draw_contours(img, filtered_contours)
    
    # Match shapes
    matches = [( match_shape(cnt), find_centroid(cnt) ) for cnt in filtered_contours]

    # Draw centroids
    img_centroids = draw_centroids(img, [m[1] for m in matches])

    # Write to file
    write_to_file(matches)

    # Draw shape matches
    img_shapes = img.copy()
    for m in matches:
        x,y = m[1]
        cv2.putText(img_shapes, m[0], (x-25, y), cv2.FONT_ITALIC, 0.4, (0,255,0))
   
    # Display images
    #cv2.imshow("shapes", img_shapes)
    #cv2.imshow("asdasda", contoured_img)
    #cv2.waitKey()

