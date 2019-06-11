import pika
from pika.credentials import ExternalCredentials
import time
import ssl
import json
from pymisp import PyMISP
from keys import misp_url, misp_key

import re
import os

def check_ip4_version(ip):
    a = ip.split('.')
    if len(a) != 4:
        return False
    for x in a:
        if not x.isdigit():
            return False
        i = int(x)
        if i < 0 or i > 255:
            return False
    return True

def init(url, key):
    return PyMISP(url, key, False, 'json', debug=False)

# UPC
def fixuprere(adict, fre, vre, sre):
    p = re.compile(fre) # compile the key pattern
    q = re.compile(vre) # compile de value pattern
    for key in adict.keys(): # For each key in dict
        m=p.match(key) # look for a match in the key
        if m: #  if there is a MATCH in the key
            n=q.match(adict[key]) 
            if n: # if MATCH in the value
                adict[key] = q.sub(sre, adict[key]) # replace value with sre regexp (e.g. '\\1\\2X.X')             
        elif type(adict[key]) is dict: # If element is a dict
            fixuprere(adict[key], fre, vre, sre)
        elif type(adict[key]) is list: # if element is a list
            for d in adict[key]:
                fixuprere(d, fre, vre, sre) 

# Recorre toda la estructura de log de CIPSEC, 3 niveles de diccionarios
def anonimize(event, kre, vre, sre):
    kpattern = re.compile(kre, re.IGNORECASE) # compile the key pattern
    vpattern = re.compile(vre) # compile the value pattern
    for k in event.keys(): # look for matches in the keys of the first level
	mkey=kpattern.match(k) # look for a match in the key (i means case insensitive)
	if mkey: #  if there is a MATCH in the key
	    mvalue=vpattern.match(event[k]) # look if the value matches with vpattern
            if mvalue: # if MATCH in the value
	    	event[k] = vpattern.sub(sre, event[k]) # replace value with sre regexp (e.g. '\\1\\2X.X') 
		#print(event[k])            
	elif type(event[k]) is dict: # identify RELATED_EVENTS_INFO
	    print(k)
	    anonimize(event[k], kre, vre, sre)
	    for k1 in event[k].keys():
		if type(event[k][k1]) is dict: # a:, b: in RELATED_EVENTS_INFO
		    anonimize(event[k][k1], kre, vre, sre)
		    #print(k1)
    #print(event)

def anon(policies_file, event):
    with open(policies_file, 'r') as f: # json file is loaded as a list of dicts (structure DEPENDS on the file)
        policies = json.load(f) # archivo_dict is a list of dicts
    
    for kpolicy in policies.keys():
	# Aqui se leen las politicas 
	kre = policies[kpolicy]["KRE"]
	vre = policies[kpolicy]["VRE"]
	sre = policies[kpolicy]["SRE"]
	# Aplicar cada politica
    	anonimize(event, kre, vre, sre)
    
    #print(event)


def callback(ch, method, properties, body):
    ts = time.time()
    jsonObj = json.loads(body)
    ch.basic_ack(delivery_tag=method.delivery_tag)
    print("\n [%d] Received:" % (ts, ))
    #print (jsonObj)

    #jsonw = json.dumps(jsonObj)
    #f = open("jsonObj.json","a+")
    #f.write("\r\n")
    #f.write(jsonw)
    #f.close()
    
    # @UPC - Insert your code here
    # Collect the jsonObj and anonymize the required fields. 
    #The result has to be a json with the same format as the one at jsonObj (in order to be correctly parsed by the code below)
    
    #print(type(jsonObj))
    #print(jsonObj.keys()[0])    # name of Event, always AlarmEvent according to Ruben Trapero
    #print(type(event))

    event = jsonObj.get('AlarmEvent', '')

    print("EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE")
    print(event)
    print(type(event))
    print('\n\n\n') 
    print("JSONOBJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJ")
    print(jsonObj)
    print(type(jsonObj))
    src_ip_re = '^SRC_IP$'
    dst_ip_re = '^DST_IP$'

    vre = r'(\d{1,3}\.)(\d{1,3}\.)\d{1,3}\.\d{1,3}$'
    sre = '\\1\\2X.X'

    #vpattern = re.compile(vre) # compile de value pattern

    #anonimize(event, src_ip_re, vre, sre)
    #anonimize(event, dst_ip_re, vre, sre)

    
    anon("policies.json", event)
    #print(event)
    #jsonObj = event;

    #for k in event.keys():
    #	mkey=kpattern.match(k) # look for a match in the key
    #    if mkey: #  if there is a MATCH in the key
    #	    mvalue=vpattern.match(event[k]) 
    #        if mvalue: # if MATCH in the value
    #	    	event[k] = vpattern.sub(sre, event[k]) # replace value with sre regexp (e.g. '\\1\\2X.X') 
    #		print(event[k])            
    #print(event)

    #print(event)
    
    #print(event.get('SRC_IP', None))
    #print(event.get('USERNAME', None))
    #print(event.get('ORGANIZATION', None))
    
    


    # Atos.Mario part
    # get misp instance
    misp = init(misp_url, misp_key)
    
    # conversion from XL-SIEM json to MISP json
    xlsiem_data = jsonObj.get('AlarmEvent')
    misp_data = {}
    mispEventFields = {}
    
    # basic fields addition
    if xlsiem_data.get('SID_NAME'): mispEventFields['info'] = xlsiem_data.get('SID_NAME')
        
    mispEventFields['analysis'] = 2
    mispEventFields['threat_level_id'] = 1
    if xlsiem_data.get('DATE'): 
        date = xlsiem_data.get('DATE').split()[0]
        mispEventFields['date'] = date
    
    # attributes creation
    attributes = []
    if xlsiem_data.get('SRC_IP'): 
        src_ip = {}
        src_port = {}
        src_ip['category'] = "Network activity"
        src_ip['type'] = 'ip-src'
        src_ip['to_ids'] = False 
        src_ip['value'] = xlsiem_data.get('SRC_IP')
        src_ip['comment'] = "Source IP associated to the detected alarm."
        if xlsiem_data.get('SRC_PORT') and (check_ip4_version(xlsiem_data.get('SRC_IP'))):
            src_ip['type'] += '|port'
            src_ip['value'] = str(src_ip['value']) + '|' + str(xlsiem_data.get('SRC_PORT'))
            src_ip['comment'] = "Source IP and port associated to the detected alarm." 
        elif xlsiem_data.get('SRC_PORT'):
            src_port['category'] = "Network activity"
            src_port['type'] = 'port'
            src_port['to_ids'] = False 
            src_port['value'] = xlsiem_data.get('SRC_PORT')
            src_port['comment'] = "Source Port associated to the detected alarm."
            attributes.append(src_port)
        attributes.append(src_ip)
        
    if xlsiem_data.get('DST_IP'): 
        dst_ip = {}
        dst_port = {}
        dst_ip['category'] = "Network activity"
        dst_ip['type'] = 'ip-dst'
        dst_ip['to_ids'] = False 
        dst_ip['value'] = xlsiem_data.get('DST_IP')
        dst_ip['comment'] = "Destination IP associated to the detected alarm."
        if xlsiem_data.get('DST_PORT') and (check_ip4_version(xlsiem_data.get('DST_IP'))):
            dst_ip['type'] += '|port'
            dst_ip['value'] = str(dst_ip['value']) + '|' + str(xlsiem_data.get('DST_PORT'))
            dst_ip['comment'] = "Destination IP and port associated to the detected alarm."
        elif xlsiem_data.get('DST_PORT'):
            dst_port['category'] = "Network activity"
            dst_port['type'] = 'port'
            dst_port['to_ids'] = False 
            dst_port['value'] = xlsiem_data.get('DST_PORT')
            dst_port['comment'] = "Destination Port associated to the detected alarm."
            attributes.append(dst_port) 
        attributes.append(dst_ip)
        
    if xlsiem_data.get('RISK'): 
        risk = {}
        risk['category'] = "External analysis"
        risk['type'] = 'other'
        risk['to_ids'] = False 
        risk['value'] = xlsiem_data.get('RISK')
        risk['comment'] = "Risk value evaluated by XL-SIEM"
        attributes.append(risk)
        
    if xlsiem_data.get('PRIORITY'): 
        priority = {}
        priority['category'] = "External analysis"
        priority['type'] = 'other'
        priority['to_ids'] = False 
        priority['value'] = xlsiem_data.get('PRIORITY')
        priority['comment'] = "Priority value evaluated by XL-SIEM"
        attributes.append(priority)
    
    if xlsiem_data.get('RELIABILITY'): 
        reliability = {}
        reliability['category'] = "External analysis"
        reliability['type'] = 'other'
        reliability['to_ids'] = False 
        reliability['value'] = xlsiem_data.get('RELIABILITY')
        reliability['comment'] = "Reliability value evaluated by XL-SIEM"
        attributes.append(reliability)
        
    if xlsiem_data.get('USERDATA1'): 
        userdata1 = {}
        userdata1['category'] = "Other"
        userdata1['type'] = 'other'
        userdata1['to_ids'] = False 
        userdata1['value'] = xlsiem_data.get('USERDATA1')
        userdata1['comment'] = "Userdata1"
        attributes.append(userdata1)
    
    if xlsiem_data.get('USERDATA2'): 
        userdata2 = {}
        userdata2['category'] = "Other"
        userdata2['type'] = 'other'
        userdata2['to_ids'] = False 
        userdata2['value'] = xlsiem_data.get('USERDATA2')
        userdata2['comment'] = "Userdata2"
        attributes.append(userdata2)
    
    if xlsiem_data.get('USERDATA3'): 
        userdata3 = {}
        userdata3['category'] = "Other"
        userdata3['type'] = 'other'
        userdata3['to_ids'] = False 
        userdata3['value'] = xlsiem_data.get('USERDATA3')
        userdata3['comment'] = "Userdata3"
        attributes.append(userdata3)
    
    if xlsiem_data.get('USERDATA4'): 
        userdata4 = {}
        userdata4['category'] = "Other"
        userdata4['type'] = 'other'
        userdata4['to_ids'] = False 
        userdata4['value'] = xlsiem_data.get('USERDATA4')
        userdata4['comment'] = "Userdata4"
        attributes.append(userdata4)
    
    if xlsiem_data.get('USERDATA5'): 
        userdata5 = {}
        userdata5['category'] = "Other"
        userdata5['type'] = 'other'
        userdata5['to_ids'] = False 
        userdata5['value'] = xlsiem_data.get('USERDATA5')
        userdata5['comment'] = "Userdata5"
        attributes.append(userdata5)
    
    if xlsiem_data.get('USERDATA6'): 
        userdata6 = {}
        userdata6['category'] = "Other"
        userdata6['type'] = 'other'
        userdata6['to_ids'] = False 
        userdata6['value'] = xlsiem_data.get('USERDATA6')
        userdata6['comment'] = "Userdata6"
        attributes.append(userdata6)
    
    if xlsiem_data.get('USERDATA7'): 
        userdata7 = {}
        userdata7['category'] = "Other"
        userdata7['type'] = 'other'
        userdata7['to_ids'] = False 
        userdata7['value'] = xlsiem_data.get('USERDATA7')
        userdata7['comment'] = "Userdata7"
        attributes.append(userdata7)
        
    if xlsiem_data.get('USERDATA8'): 
        userdata8 = {}
        userdata8['category'] = "Other"
        userdata8['type'] = 'other'
        userdata8['to_ids'] = False 
        userdata8['value'] = xlsiem_data.get('USERDATA8')
        userdata8['comment'] = "Userdata8"
        attributes.append(userdata8)
        
    if xlsiem_data.get('USERDATA9'): 
        userdata9 = {}
        userdata9['category'] = "Other"
        userdata9['type'] = 'other'
        userdata9['to_ids'] = False 
        userdata9['value'] = xlsiem_data.get('USERDATA9')
        userdata9['comment'] = "Userdata9"
        attributes.append(userdata9)
    
    mispEventFields['Attribute'] = attributes
    
    # tags creation
    tags = []
    
    
    if xlsiem_data.get('CATEGORY'): 
        categoryTag = {}
        categoryTag['color'] = '#0000FF'
        categoryTag['exportable'] = 'false'
        category = xlsiem_data.get('CATEGORY').lower()
        category.replace("_", "-")
        categoryTag['name'] = 'xl-siem:category=' + category
        tags.append(categoryTag)
    
    if xlsiem_data.get('SUBCATEGORY'): 
        subcategoryTag = {}
        subcategoryTag['color'] = '##00FFFF'
        subcategoryTag['exportable'] = 'false'
        subcategory = xlsiem_data.get('SUBCATEGORY').lower()
        subcategory.replace("_", "-")
        subcategoryTag['name'] = 'xl-siem:sub-category=' + subcategory
        tags.append(subcategoryTag)
        
    mispEventFields['Tag'] = tags
    
    misp_data['Event'] = mispEventFields
    
    json_data = json.dumps(misp_data)
    
    #print(json_data)

    final_event = misp.add_event(json_data)
    #event = misp.get_event(1)
    
    #final_event = misp.get_stix_event(15, False, False, False, False)
    #print(final_event)

HOST ='212.34.151.202'
SSL_PORT = 5671

SSL_OPTIONS = dict(
        ssl_version=ssl.PROTOCOL_TLSv1_1,
        ca_certs="./cacert.pem",
        keyfile="./key.pem",
        certfile="./cert.pem",
        cert_reqs=ssl.CERT_REQUIRED)

credentials = ExternalCredentials()
connection = pika.BlockingConnection(pika.ConnectionParameters(host=HOST, port=SSL_PORT, ssl=True, ssl_options=SSL_OPTIONS,
        socket_timeout=0.25, credentials=credentials))

channel = connection.channel()
channel.queue_declare(queue='eu.cipsec.dw_input', durable=True)

consumer_tag = channel.basic_consume(callback, queue='eu.cipsec.dw_input', no_ack=False, exclusive=False)
print(' [*] Waiting for messages. To exit press CTRL+C')
channel.start_consuming()

connection.close()

