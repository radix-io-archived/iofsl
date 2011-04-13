# Parses the node list. Returns a dictionary containing node names and 
# processor counts
def ParseNodeFile (nodelist):
  ret = {}
  for x in nodelist:
    if "\n" in x:
      x = x[:-1]
    if x in ret:
      ret[x] += 1
    else:
      ret[x] = 1
  return ret

