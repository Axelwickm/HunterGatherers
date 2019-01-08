import random
from numpy.random import choice
import json

markovChain = {}
lookback = 4

def addToChain(a, b):
    #print(repr(a+" -> "+b))
    if a not in markovChain:
        markovChain[a] = {}
    if b not in markovChain[a]:
        markovChain[a][b] = 0
    markovChain[a][b] += 1


def processName(name):
    addToChain("START", name[0])
    for x in range(1, len(name)):
        addToChain(name[max(x-lookback, 0):x], name[x])

def generate():
    last = "START"
    name = ""
    while True:
        try:
            last = choice(list(markovChain[last].keys()), 1, list(markovChain[last].values()))[0]
        except KeyError:
            last = last[1:]
            if last == "":
                name += "\n"
                break
            continue
        name += last
        last = name[max(len(name)-lookback, 0): len(name)]
        if name[len(name)-1:len(name)] == "\n":
            break


    return name


f = open("surnames.txt", encoding="utf8")
for line in f:
    if line.rstrip() != "":
        processName(line.split(",")[0]+"\n")


markovChainConverted = {}

for a, bList in markovChain.items():
    markovChainConverted[a] = list(bList.items())
    markovChainConverted[a].sort(key=lambda x: x[1], reverse = True)

with open('NamesMarkov.json', 'w') as outfile:
    json.dump(markovChainConverted, outfile, indent=0)
    # This works, but file is 3 times larger than it has to be.
    # Therefore the generated repo file has been minified.

print("File saved.")

for i in range(1):
    print(repr(generate()))