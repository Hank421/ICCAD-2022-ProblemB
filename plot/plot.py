import matplotlib.pyplot as plt
import matplotlib.patches as pat
import random
import math

inputFileName = 'module.txt'
outputFileName = 'placement.png'

fig = plt.figure()
ax = fig.add_subplot(111)

bdy_list = []
f_id = 0

minX = math.inf
minY = math.inf
maxX = 0
maxY = 0

with open(inputFileName, 'r') as f:    
    for line in f:
        f_id += 1
        if f_id == 1:
            bdy_list = line.split()
            for i in range(4):
                bdy_list[i] = float(bdy_list[i])
            minX, minY, maxX, maxY = bdy_list[0], bdy_list[1], bdy_list[2], bdy_list[3]            
            continue
        coor = line.split()        
        idxs = coor[1:]
        for i in range(4):
            idxs[i] = (float(idxs[i]))        
        
        minX = min(minX, idxs[0])
        minY = min(minY, idxs[1])
        maxX = max(maxX, idxs[2])
        maxY = max(maxY, idxs[3])

        r = random.uniform(0.6, 1)
        g = random.uniform(0.6, 1)
        b = random.uniform(0.6, 1)
        color = (r, g, b)
        
        rect = pat.Rectangle((idxs[0], idxs[1]), idxs[2] - idxs[0], idxs[3] - idxs[1], edgecolor='black', facecolor = color)
        ax.add_patch(rect)        
        # plt.annotate(coor[0], ((idxs[0] + idxs[2]) / 2, (idxs[1] + idxs[3]) / 2), ha='center', fontsize=10)

# Boundary outline
rect = pat.Rectangle((bdy_list[0], bdy_list[1]), bdy_list[2] - bdy_list[0], bdy_list[3] - bdy_list[1], edgecolor='red', facecolor='None')
ax.add_patch(rect)

# a, b, c = 5, 3, 8
# rect = pat.Rectangle(((a * bdy_list[0] + b * bdy_list[2]) / c, (a * bdy_list[1] + b * bdy_list[3]) / c), (bdy_list[2] - bdy_list[0]) * 2 / c, (bdy_list[3] - bdy_list[1]) * 2 / c, edgecolor='red', facecolor='None')
# ax.add_patch(rect)

ax.set_aspect((maxY - minY + 200) / (maxX - minX + 200))
plt.xlim([minX - 100, maxX + 100])
plt.ylim([minY - 100, maxY + 100])
plt.tight_layout()

fig.set_size_inches(10, 10) # set figure size
# plt.show()
plt.savefig(outputFileName, dpi=300) # save figure