
import pika
from pika.credentials import ExternalCredentials
import time
import ssl
import json
from pymisp import PyMISP
from keys import misp_url, misp_key


import re
import os
import hashlib

import argparse # UPC: To read the policy file as an argument

# UPC: Parse the argument policy_file, received when executing the script
parser = argparse.ArgumentParser()
parser.add_argument("policy_file")
parser.add_argument("scope")
args = parser.parse_args()
print(args.policy_file)
print(args.scope)
policies_file = args.policy_file # this variable contains the argument provided when executing the script
scope = args.scope
scope = scope.lower() # to lower case

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

# UPC functions

# anonymizeEvent: Anonymize a single event
# having key, value and substitution regular expressions
# 3-level dictionaries are considered
# CIPSEC provides a JSON object for each generated event. This object is structured as a python dictionary.
# The elements of this dictionary could also be a dictionary.
# This function anonymize this JSON object and returns the anonymized object.

def anonymizeEvent(event, kre, vre, sre):
    kpattern = re.compile(kre, re.IGNORECASE) # compile the key pattern
    vpattern = re.compile(vre) # compile the value pattern
    for k in event.keys(): # look for matches in the keys of the first level
        mkey=kpattern.match(k) # look for a match in the key (i means case insensitive)
        if mkey: #  if there is a MATCH in the key
            if event[k] is None: # some fields could have no data, so we put a string and then anonimyze
                event[k] = 'NoData'
            mvalue=vpattern.match(event[k]) # look if the value matches with vpattern
            if mvalue: # if MATCH in the value
                event[k] = vpattern.sub(sre, event[k]) # replace value with sre regexp (e.g. '\\1\\2X.X')
                #znt[k])
        elif type(event[k]) is dict: # identify RELATED_EVENTS_INFO field (is a dict)
            anonymizeEvent(event[k], kre, vre, sre)
            for k1 in event[k].keys(): # each element of RELATED_EVENTS_INFO is another event
                if type(event[k][k1]) is dict: # a:, b: in RELATED_EVENTS_INFO, each element could be a dict
                    anonymizeEvent(event[k][k1], kre, vre, sre)
                elif type(event[k][k1]) is list: # a: b: in RELATED_EVENTS_INFO could also be a list of dicts
                    for a in event[k][k1]:
                        anonymizeEvent(a, kre, vre, sre)

# Anon: Anonymize an event according to the policies in a json file

# an anonymization policy (or policies) is loaded through a file
# each policy is represented as a dictionary that stores the anonymization parameters
# policies file is a list of dictionaries

def anon(policies_file, event):
    with open(policies_file, 'r') as f: # json file is loaded as a list of dicts (structure DEPENDS on the file)
        policies = json.load(f) # archivo_dict is a list of dicts

    for kpolicy in policies.keys(): # for each policy in the file
        # Load porganization, if defined, from the policy file (porganization identifies the org whose logs will be anonimyzed)
	# But, due to CIPSEC requirements, this has to be defined by the scope
        #porganization = policies[kpolicy]["ORGANIZATION"] # porganization (scope is passed by the dashboard as script argument)
        #porganization = scope # porganization (scope is passed by the dashboard as a script argument) see scope in the first lines
        # does scope (porganization) match with the organization of the log or is it been used the wildcard all?
        if scope == event.get('ORGANIZATION', '') or scope == "all" : # otherwise, nothing is done
                kre = policies[kpolicy]["KRE"]

                paction =  policies[kpolicy].get('ACTION') # if paction in policies file is not set, returns None = False
                if paction == "suppression": # if a paction is defined as suppression
                        vre = '.*' # no matter the value, anonimyze it
                        sre = '**********' # suppress using ***
                        anonymizeEvent(event, kre, vre, sre)
                elif paction == "generalization":
                        if  policies[kpolicy].get('TYPE') == "ip_address": # if policy applies for the ip address field
                                vre = '(\\d{1,3}\\.)(\\d{1,3}\\.)\\d{1,3}\\.\\d{1,3}$' #
                                sre = '\\1\\2X.X' # this is one way to generalize it
                                anonymizeEvent(event, kre, vre, sre)
                        if policies[kpolicy].get('TYPE') == "organization": # if policy applies for the organization field
                                #if scope == 'csi' or scope == 'db' or scope == 'hospital' or scope == 'proto':
                                vre = '.*' #
                                sre = "organization"
                                anonymizeEvent(event, kre, vre, sre)
                #elif paction == "pseudonimization":
                #        vre = '.*'
                #        if  policies[kpolicy].get('TYPE') == "organization": # If pseudonimizing an organization (csi, db, hospital)
                #                if event.get('ORGANIZATION', '') == 'csi':
                #                        sre = 'pilot1' # assign a pseudonim
                #                        anonymizeEvent(event, kre, vre, sre)
		#		elif event.get('ORGANIZATION', '') == 'db':
                #                        sre = 'pilot2'
                #                        anonymizeEvent(event, kre, vre, sre)
                #                elif event.get('ORGANIZATION', '') == 'hospital':
                #                        sre = 'pilot3'
                #                        anonymizeEvent(event, kre, vre, sre)
		#		elif event.get('ORGANIZATION', '') == 'proto':
                #                        sre = 'pilot4'
                #                        anonymizeEvent(event, kre, vre, sre)
                #                else:
                #                        sre = 'unknown_pilot'
                #                        anonymizeEvent(event, kre, vre, sre)
		elif paction == "pseudonimization":
			vre = '.*'
			organization = event.get('ORGANIZATION', '')
			sre = organization
			hash_object = hashlib.md5(organization)
			sre = hash_object.hexdigest() # sustitution is hash the content of ORG (encode transforms string to bytes)
			anonymizeEvent(event, kre, vre, sre)

                else:
                        kre = policies[kpolicy]["KRE"]
                        vre = policies[kpolicy]["VRE"]
                        sre = policies[kpolicy]["SRE"]
                        print(kre, vre, sre)
                        # Apply the policy
                        anonymizeEvent(event, kre, vre, sre)

#######################################################################


def init(url, key):
    return PyMISP(url, key, False, 'json', debug=False)

def callback(ch, method, properties, body):
    ts = time.time()
    jsonObj = json.loads(body)
    ch.basic_ack(delivery_tag=method.delivery_tag)

    # @UPC - Insert your code here
    # Collect the jsonObj and anonymize the required fields.
    #The result has to be a json with the same format as the one at jsonObj (in order to be correctly parsed by the code below)

    #UPC: Anonymize the received event based on the policies taken from file policies.json

    event = jsonObj.get('AlarmEvent', '') # get the content of the event

    #print("\n [%d] Received:" % (ts, ))
    #print (body)
    print("CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC")
    print(jsonObj)
    print("EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE")
    # UPC: Anonimyze the event according to the parameters in policies_file
    anon(policies_file, event) # policies_file is obtained at the begining of this script from the command line
    ## UPC: Substitute the original jsonObj with the anonymized event
    jsonObj = {u'AlarmEvent':event}

    print('\n\n\n')
    print("JJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJSONOBJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJ")
    print(jsonObj) # now anonimyzed
    print("EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE")




    # Mario part
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
    mispEventFields['distribution'] = 0
    if xlsiem_data.get('DATE'):
        date = xlsiem_data.get('DATE').split()[0]
        mispEventFields['date'] = date

    # attributes creation
    attributes = []
    if xlsiem_data.get('SRC_IP'):
        src_ip = {}
        src_port = {}
        src_ip['category'] = "Network activity"
        src_ip['type'] = 'other'
        src_ip['to_ids'] = False
        src_ip['value'] = xlsiem_data.get('SRC_IP')
        src_ip['disable_correlation'] = True
        src_ip['comment'] = "Source IP associated to the detected alarm."
        if xlsiem_data.get('SRC_PORT') and (check_ip4_version(xlsiem_data.get('SRC_IP'))):
            src_ip['type'] = 'ip-src|port'
            src_ip['value'] = str(src_ip['value']) + '|' + str(xlsiem_data.get('SRC_PORT'))
            src_ip['comment'] = "Source IP and port associated to the detected alarm."
        elif xlsiem_data.get('SRC_PORT'):
            src_port['category'] = "Network activity"
            src_port['type'] = 'port'
            src_port['to_ids'] = False
            src_port['value'] = xlsiem_data.get('SRC_PORT')
            src_port['comment'] = "Source Port associated to the detected alarm."
            src_port['disable_correlation'] = True
            attributes.append(src_port)
        attributes.append(src_ip)

    if xlsiem_data.get('DST_IP'):
        dst_ip = {}
        dst_port = {}
        dst_ip['category'] = "Targeting data"
        dst_ip['type'] = 'target-machine'
        dst_ip['to_ids'] = False
        dst_ip['value'] = xlsiem_data.get('DST_IP')
        dst_ip['comment'] = "Destination IP associated to the detected alarm."
        dst_ip['disable_correlation'] = True
        if xlsiem_data.get('DST_PORT') and (check_ip4_version(xlsiem_data.get('DST_IP'))):
            dst_ip['category'] = "Network activity"
            dst_ip['type'] = 'ip-dst|port'
            dst_ip['value'] = str(dst_ip['value']) + '|' + str(xlsiem_data.get('DST_PORT'))
            dst_ip['comment'] = "Destination IP and port associated to the detected alarm."
        elif xlsiem_data.get('DST_PORT'):
            dst_port['category'] = "Targeting data"
            dst_port['type'] = 'target-location'
            dst_port['to_ids'] = False
            dst_port['value'] = xlsiem_data.get('DST_PORT')
            dst_port['comment'] = "Destination Port associated to the detected alarm."
            dst_port['disable_correlation'] = True
            attributes.append(dst_port)
        attributes.append(dst_ip)

    if xlsiem_data.get('RISK'):
        risk = {}
        risk['category'] = "External analysis"
        risk['type'] = 'other'
        risk['to_ids'] = False
        risk['value'] = xlsiem_data.get('RISK')
        risk['comment'] = "Risk value evaluated by XL-SIEM"
        risk['disable_correlation'] = True
        attributes.append(risk)

    if xlsiem_data.get('PRIORITY'):
        priority = {}
        priority['category'] = "External analysis"
        priority['type'] = 'other'
        priority['to_ids'] = False
        priority['value'] = xlsiem_data.get('PRIORITY')
        priority['comment'] = "Priority value evaluated by XL-SIEM"
        priority['disable_correlation'] = True
        attributes.append(priority)

    if xlsiem_data.get('ORGANIZATION'):
        organization = {}
        organization['category'] = "Internal reference"
        organization['type'] = 'other'
        organization['to_ids'] = False
        organization['value'] = xlsiem_data.get('ORGANIZATION')
        organization['comment'] = "Organization where the XL-SIEM Agent has been deployed"
        organization['disable_correlation'] = True
        attributes.append(organization)

    if xlsiem_data.get('USERNAME'):
        username = {}
        username['category'] = "Person"
        username['type'] = 'other'
        username['to_ids'] = False
        username['value'] = xlsiem_data.get('USERNAME')
        username['comment'] = "Username associated with the detected event"
        username['disable_correlation'] = True
        attributes.append(username)

    if xlsiem_data.get('RELIABILITY'):
        reliability = {}
        reliability['category'] = "External analysis"
        reliability['type'] = 'other'
        reliability['to_ids'] = False
        reliability['value'] = xlsiem_data.get('RELIABILITY')
        reliability['comment'] = "Reliability value evaluated by XL-SIEM"
        reliability['disable_correlation'] = True
        attributes.append(reliability)

    if xlsiem_data.get('USERDATA1'):
        userdata1 = {}
        userdata1['category'] = "Other"
        userdata1['type'] = 'other'
        userdata1['to_ids'] = False
        userdata1['value'] = xlsiem_data.get('USERDATA1')
        userdata1['comment'] = "Userdata1"
        userdata1['disable_correlation'] = True
        attributes.append(userdata1)

    if xlsiem_data.get('USERDATA2'):
        userdata2 = {}
        userdata2['category'] = "Other"
        userdata2['type'] = 'other'
        userdata2['to_ids'] = False
        userdata2['value'] = xlsiem_data.get('USERDATA2')
        userdata2['comment'] = "Userdata2"
        userdata2['disable_correlation'] = True
        attributes.append(userdata2)

    if xlsiem_data.get('USERDATA3'):
        userdata3 = {}
        userdata3['category'] = "Other"
        userdata3['type'] = 'other'
        userdata3['to_ids'] = False
        userdata3['value'] = xlsiem_data.get('USERDATA3')
        userdata3['comment'] = "Userdata3"
        userdata3['disable_correlation'] = True
        attributes.append(userdata3)

    if xlsiem_data.get('USERDATA4'):
        userdata4 = {}
        userdata4['category'] = "Other"
        userdata4['type'] = 'other'
        userdata4['to_ids'] = False
        userdata4['value'] = xlsiem_data.get('USERDATA4')
        userdata4['comment'] = "Userdata4"
        userdata4['disable_correlation'] = True
        attributes.append(userdata4)

    if xlsiem_data.get('USERDATA5'):
        userdata5 = {}
        userdata5['category'] = "Other"
        userdata5['type'] = 'other'
        userdata5['to_ids'] = False
        userdata5['value'] = xlsiem_data.get('USERDATA5')
        userdata5['comment'] = "Userdata5"
        userdata5['disable_correlation'] = True
        attributes.append(userdata5)

    if xlsiem_data.get('USERDATA6'):
        userdata6 = {}
        userdata6['category'] = "Other"
        userdata6['type'] = 'other'
        userdata6['to_ids'] = False
        userdata6['value'] = xlsiem_data.get('USERDATA6')
        userdata6['comment'] = "Userdata6"
        userdata6['disable_correlation'] = True
        attributes.append(userdata6)

    if xlsiem_data.get('USERDATA7'):
        userdata7 = {}
        userdata7['category'] = "Other"
        userdata7['type'] = 'other'
        userdata7['to_ids'] = False
        userdata7['value'] = xlsiem_data.get('USERDATA7')
        userdata7['comment'] = "Userdata7"
        userdata7['disable_correlation'] = True
        attributes.append(userdata7)

    if xlsiem_data.get('USERDATA8'):
        userdata8 = {}
        userdata8['category'] = "Other"
        userdata8['type'] = 'other'
        userdata8['to_ids'] = False
        userdata8['value'] = xlsiem_data.get('USERDATA8')
        userdata8['comment'] = "Userdata8"
        userdata8['disable_correlation'] = True
        attributes.append(userdata8)

    if xlsiem_data.get('USERDATA9'):
        userdata9 = {}
        userdata9['category'] = "Other"
        userdata9['type'] = 'other'
        userdata9['to_ids'] = False
        userdata9['value'] = xlsiem_data.get('USERDATA9')
        userdata9['comment'] = "Userdata9"
        userdata9['disable_correlation'] = True
        attributes.append(userdata9)

    mispEventFields['Attribute'] = attributes

    # tags creation
    tags = []


    if xlsiem_data.get('CATEGORY'):
        categoryTag = {}
        categoryTag['color'] = '#0000FF'
        categoryTag['exportable'] = 'false'
        category = xlsiem_data.get('CATEGORY').lower()
        modified_category = category.replace("_", "-")
        categoryTag['name'] = 'xl-siem:category=' + "\"" + modified_category + "\""
        tags.append(categoryTag)

    if xlsiem_data.get('SUBCATEGORY'):
        subcategoryTag = {}
        subcategoryTag['color'] = '##00FFFF'
        subcategoryTag['exportable'] = 'false'
        subcategory = xlsiem_data.get('SUBCATEGORY').lower()
        modified_subcategory = subcategory.replace("_", "-")
        subcategoryTag['name'] = 'xl-siem:sub-category=' + "\"" + modified_subcategory + "\""
        tags.append(subcategoryTag)

    mispEventFields['Tag'] = tags

    misp_data['Event'] = mispEventFields

    json_data = json.dumps(misp_data)
    print("----------------------------------JSON DATA -------------------------------")
    print(json_data)
    print("---------------------------END JSON DATA -------------------------------")
    final_event = misp.add_event(json_data)
    #event = misp.get_event(1)

    #final_event = misp.get_stix_event(15, False, False, False, False)
    print("---------------------------FINAL EVENT -------------------------------")
    print(final_event)
    print("--------------------------- END FINAL EVENT -------------------------------")

HOST ='212.34.151.202'
SSL_PORT = 5671

SSL_OPTIONS = dict(
        ssl_version=ssl.PROTOCOL_TLSv1_1,
        ca_certs="./cacert.pem",
        keyfile="./key.pem",
        certfile="./cert.pem",
        cert_reqs=ssl.CERT_REQUIRED)

credentials = ExternalCredentials()

while True:
    try:
        connection = pika.BlockingConnection(pika.ConnectionParameters(host=HOST, port=SSL_PORT, ssl=True, ssl_options=SSL_OPTIONS, socket_timeout=0.25, credentials=credentials))

        channel = connection.channel()
        channel.queue_declare(queue='eu.cipsec.dw_input', durable=True)

        consumer_tag = channel.basic_consume(callback, queue='eu.cipsec.dw_input', no_ack=False, exclusive=False)
        print(' [*] Waiting for messages. To exit press CTRL+C')
        channel.start_consuming()
        connection.close()
    #except pika.exceptions.ConnectionClosed, e:
    except Exception, e:
        print("Connection error. Trying again...")
        time.sleep(5)
