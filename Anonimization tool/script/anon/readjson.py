import json
import os
import re
import pprint


with open('policies.json', 'r') as f: # json file is loaded as a list of dicts (structure DEPENDS on the file)
    policies = json.load(f) # archivo_dict is a list of dicts

for kpolicy in policies.keys():
	# Aqui se leen las politicas 
	sre = policies[kpolicy]["SRE"] # Aplicar cada politica
	vre = policies[kpolicy]["VRE"]
	kre = policies[kpolicy]["KRE"]
	print(vre)

#keyre = 'IP.*'
#vre = r'(\d{1,3}\.)(\d{1,3}\.)\d{1,3}\.\d{1,3}$'
#sre = '\\1\\2X.X'
#sre = 'XXXX'

	

#policie_srcip = policies.get('SRCIP', '')
#policie_srcip = policies.get('SRCIP', '')
#print(policie_srcip["SRE"])


