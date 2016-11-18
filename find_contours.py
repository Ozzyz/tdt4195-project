import numpy as np
import cv2
import matplotlib import pyplot as plt

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
