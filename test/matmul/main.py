#!/usr/bin/python3

import ane
import numpy as np

model = ane.model("matmul.anec")

x1 = np.random.random((1, 1, 2, 3)).astype(np.float16) 
x2 = np.random.random((1, 1, 3, 2)).astype(np.float16) 
pred = model.predict([x1, x2])
ref = x1 @ x2
print(pred - ref)
