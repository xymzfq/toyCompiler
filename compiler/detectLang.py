import re
import sys
print(sys.argv[1])
f = open(sys.argv[1],'r')
code = str(f.read())
code = code.replace(' ','')
code = code.replace('\n','')
code = code.replace('	','')
#print (code)
#code = "int"
pattern = re.compile(r"(.*)struct(.*)")
match = pattern.match(code)
fo = open('lang', 'w') 

if match:
    fo.write('c')
else:
    pattern = re.compile(r'(.*)begin(.*)')
    match = pattern.match(code)
    if match:
        fo.write('psc')
    else:
        pattern = re.compile(r'(.*)end(.*)')
        match = pattern.match(code)
        if match:
            fo.write('lua')
        else:
            fo.write('js')

f.close()
fo.close()
