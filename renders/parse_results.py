import sys

def process(file):
   params = {}
   for line in file:
      stripped = line.strip()
      try:
         key,value = stripped.split(":")
         params[key]=value
      except ValueError:
         # print("Failed to parse '{0}'".format(stripped))
         pass

   return params

if __name__ == '__main__':
   if len(sys.argv) < 2:
      print("Usage: python parse_results.py <results.txt>")
      sys.exit(0)

   params = {}

   with open(sys.argv[1], "r") as f:
      params = process(f)

   for k,v in params.items():
      print("{0} = {1}".format(k, v))