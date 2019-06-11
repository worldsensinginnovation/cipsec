var express = require('express');

var scriptDir = '/home/ares/webserver/anon-tool/script/';
var scriptPR = 'anon-service-PR.sh';

var bodyParser = require('body-parser');
var request = require('request');
var server = express();
var exec = require('child_process').exec;

server.use(bodyParser.urlencoded({ extended: false }));
server.use(bodyParser.json());
server.use(respCorsHeaders);
server.use(errorHandler);

server.post('/', function (req, resp, next) {

    var token = req.headers['token'];
    var username = req.headers['username'];
    var scopes = req.headers['scopes'];
    var metafile = req.headers['metafile'];
    var action = req.headers['action'];

    if(token == null || metafile == null || username == null || scopes == null || action == null){
        next('missing headers');
    }

    validateClient(token, username, scopes).then(function (isValid) {
        if(isValid){
            switch (action) {
                case 'status':
                    statusCheck(scopes.split(',')[0], next).then(function (exitCode) {
                        console.log('Status check ['+scopes+']'+ exitCode);
                        if (exitCode) {
                            resp.status(200).json({message: "Running"});
                        } else {
                            resp.status(200).json({message: "Stopped"});
                        }
                    });
                    break;
                case 'start':
                    start(metafile, scopes.split(',')[0], next).then(function (exitCode) {
                        console.log('Start ['+scopes+']'+ exitCode);
                        if (exitCode) {
                            resp.status(200).json({message: "Running"});
                        } else {
                            resp.status(500).json({message: "Crash"});
                        }
                    });
                    break;
                case 'stop':
                    stop(scopes.split(',')[0], next).then(function (exitCode) {
                        console.log('Stop ['+scopes+']'+ exitCode);
                        if (exitCode) {
                            resp.status(200).json({message: "Stopped"});
                        } else {
                            resp.status(500).json({message: "Crash"});
                        }
                    });
                    break;
            }
        } else {
            console.log("not validated " + isValid);
            next('unauthorized');
        }
    })
});

module.exports = server;

function statusCheck(scope, next) {
    return new Promise(function(resolve) {
        execCommand('status', '', scope).then(function (status) {
            console.log('status ['+scope+'] '+status);
            if (status.substring(status.lastIndexOf(" ")+1) === 'running.\n'){
                resolve(true);
            } else if (status.substring(status.lastIndexOf(" ")+1) === 'failed!\n') {
                resolve(false);
            } else {
                next('bad request')
            }
        })
    });
}

function start(file, scope, next) {
    return new Promise(function(resolve) {
        execCommand('start', file, scope).then(function (status) {
            console.log('start ['+scope+']'+status);
            if (status.substring(status.lastIndexOf(" ")+1) === 'daemon:.\n') {
                resolve(true);
            } else {
                resolve(false);
            }
        })
    });
}

function stop(scope, next) {
    return new Promise(function (resolve) {
        execCommand('stop', '', scope).then(function (status) {
            console.log('stop ['+scope+']'+status);
            if (status.substring(status.lastIndexOf(" ")+1) === 'daemon:.\n') {
                resolve(true);
            } else {
                resolve(false);
            }
        })
    })
}

function execCommand(action, file, scope) {
    return new Promise(function (resolve) {
        switch (scope) {
            case 'proto':
                exec(scriptDir+scriptPR+' '+action+' '+file,
                    function (error, stdout, stderr) {
                        resolve(stdout)
                    });
                break;
            default:
                resolve('error')
        }
    });
}

function validateClient(token, username, scopes) {
    return new Promise((resolve) => {
        const validationUrl = 'https://olympus.epsevg.upc.edu/oauth2/validation';
        const postData = {
            token: token,
            username: username,
            scope: scopes,
            client_id: 'client-testing-01',
            client_secret: 'client-testing-secret-6#65-'
        };

        const clientServerOptions = {
            uri: validationUrl,
            body: JSON.stringify(postData),
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            }
        };
        request(clientServerOptions, function (error, response) {
            console.log(error, response.body);
            const body = JSON.parse(response.body);
            if(body['validation'] === 'ok')
                resolve(true);
            else
                resolve(false);
        });
    });
}

function errorHandler(err, req, res, next) {
    console.error('ERROR'+err);
    switch (err) {
        case 'unauthorized':
            res.sendStatus(401);
            break;
        case 'missing headers':
            res.status(400).send(err);
            break;
        default:
            res.sendStatus(500);
    }
}

function respCorsHeaders(req, res, next) {
    res.header("Access-Control-Allow-Origin", "*");
    res.header('Access-Control-Allow-Methods', 'PUT, GET, POST, DELETE, OPTIONS');
    res.header("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
    next();
}