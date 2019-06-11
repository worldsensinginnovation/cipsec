#!/home/ares/webserver/anon-tool/script/anon/bin/python

import pika
from pika.credentials import ExternalCredentials
import time
import ssl
import json

import re
import os

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
        #porganization = policies[kpolicy]["ORGANIZATION"] # porganization (scope is passed by the dashboard as script argument)
        porganization = scope # porganization (scope is passed by the dashboard as a script argument) see scope in the first lines
        # does scope (porganization) match with the organization of the log?
        if porganization == event.get('ORGANIZATION', '') :
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
                                if porganization == 'csi' or porganization == 'db' or porganization == 'hospital':
                                        vre = '.*' #
                                        sre = "organization"
                                        anonymizeEvent(event, kre, vre, sre)
                elif paction == "pseudonimization":
                        vre = '.*'
                        if  policies[kpolicy].get('TYPE') == "organization": # If pseudonimizing an organization (csi, db, hospital)
                                if porganization == 'csi':
                                        sre = 'pilot1' # assign a pseudonim
                                        anonymizeEvent(event, kre, vre, sre)
				elif porganization == 'db':
                                        sre = 'pilot2'
                                        anonymizeEvent(event, kre, vre, sre)
                                elif porganization == 'hospital':
                                        sre = 'pilot3'
                                        anonymizeEvent(event, kre, vre, sre)
                                else:
                                        sre = 'unkknown_pilot'
                                        anonymizeEvent(event, kre, vre, sre)

                else:
                        kre = policies[kpolicy]["KRE"]
                        vre = policies[kpolicy]["VRE"]
                        sre = policies[kpolicy]["SRE"]
                        print(kre, vre, sre)
                        # Apply the policy
                        anonymizeEvent(event, kre, vre, sre)
#######################################################################


def callback(ch, method, properties, body):
    ts = time.time()
    jsonObj = json.loads(body)
    ch.basic_ack(delivery_tag=method.delivery_tag)
    #print("\n [%d] Received:" % (ts, ))
    #print (body)
    # @UPC. ADD HERE YOUR CODE

#UPC: Anonymize the received event based on the policies taken from file policies.json

    event = jsonObj.get('AlarmEvent', '') # get the content of the event

    #print("\n [%d] Received:" % (ts, ))
    #print (body)
    print("CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC")
    print(type(jsonObj))
    print(jsonObj)
    # UPC: Anonimyze the event according to the parameters in policies_file
    anon(policies_file, event) # policies_file is obtained at the begining of this script from the command line
    ## UPC: Substitute the original jsonObj with the anonymized event
    jsonObj = {'AlarmEvent':event}

    print('\n\n\n')
    print("JJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJSONOBJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJ")
    print(type(jsonObj))
    print(jsonObj) # now anonimyzed

    jsfile = json.dumps(jsonObj)
    fp = open('anonymization_log.json','a')
    fp.write(jsfile)
    fp.write("\n")
    fp.close

#HOST ='212.34.151.202'
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

