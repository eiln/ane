#!/usr/bin/python3

import argparse
import ane
import cv2
import numpy as np

def rescale(a, low, high):
	return np.interp(a, (a.min(), a.max()), (low, high))

def preprocess(img): # (any, any, 3) RGB -> (1, 3, 512, 512)
	resized = cv2.resize(img, (512, 512), interpolation=cv2.INTER_AREA)
	transposed = np.expand_dims(resized.swapaxes(0, -1).swapaxes(1, -1), 0)
	normed = rescale(transposed, -1, +1).astype(np.float16)
	return normed

def postprocess(outarrs):# (1, 3, 2048, 2048) -> (2048, 2048, 3) RGB
	reshaped = np.swapaxes(outarrs[0].squeeze(), 0, -1).swapaxes(0, 1)
	clipped = np.round(reshaped).clip(0, 255).astype(np.uint8)
	return clipped

if __name__ == "__main__":
	parser = argparse.ArgumentParser(description='srgan')
	parser.add_argument('-l', '--lib', help='anec path', default="srgan.anec")
	requiredNamed = parser.add_argument_group('required named arguments')
	requiredNamed.add_argument('-i', '--input', help='input image', default="input.jpg")
	requiredNamed.add_argument('-o', '--output', help='output image', default="output.jpg")
	args = parser.parse_args()

	model = ane.model(args.lib)
	img = cv2.imread(args.input)[:,:,::-1]
	inarr = preprocess(img)
	outarrs = model.predict([inarr])
	pred = postprocess(outarrs)
	cv2.imwrite(args.output, pred[:,:,::-1])
