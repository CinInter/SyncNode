import unittest
import json
import unirest
import os
#os.system("some_command &")

SERVER_URL="http://127.0.0.1:5000/"

response_list=[]
login_token=[]

print("\n\nFor tests running, you should perform these actions:\
        \n-\tCompile server.c: gcc -c server.c\
        \n-\tLink server.c: gcc -o CProcess server.c\
        \n-\tLaunch CProcess (this process takes the port number as parameter): ./CProcess 51716\
        \n-\tLaunch node server: node clientnode.js\n")
try:
    input("Is your process ready? to continue press enter\n\n")
except SyntaxError:
    pass
class TestStringMethods(unittest.TestCase):

    # State : SYNC_ENDED

    def test_S_SYNC_ENDED_R_LOAD_FILE(self):
    	sucess=1
        res=unirest.post(SERVER_URL+'loadFile', headers={"Accept": "application/json"}, params={"parameter":'{"command": "LOAD_FILE testAudioFile.pcm"}',"file": open("files/testAudioFile.pcm", mode="r")})
        tmp_res = json.loads(res.raw_body)
        sucess=sucess&(tmp_res['success']==True)
        sucess=sucess&(tmp_res['data']=="FILE_FOUND_AND_LOADED")
        #res=unirest.get(SERVER_URL+'killProcess', headers={}, params={})
        res=unirest.post(SERVER_URL+'loadFile', headers={"Accept": "application/json"}, params={"parameter":'{"command": "LOAD_FILE testAudioFil.pcm"}',"file": open("files/testAudioFile.pcm", mode="r")})
        #res=unirest.post(SERVER_URL+'loadFile', headers={ "Content-Type": "application/json" }, params=json.dumps({"command": "LOAD_FILE testfil"}))
        tmp_res = json.loads(res.raw_body)
        sucess=sucess&(tmp_res['success']==False)
        sucess=sucess&(tmp_res['data']=="FILE_NOT_FOUND")
        #res=unirest.get(SERVER_URL+'killProcess', headers={}, params={})
        self.assertEqual(sucess, 1)

    # State : FILE_CHOSEN

    def test_S_FILE_CHOSEN_R_LOAD_FILE(self):
        sucess=1
        res=unirest.post(SERVER_URL+'loadFile', headers={ "Content-Type": "application/json" }, params=json.dumps({"command": "LOAD_FILE testfile"}))
        res=unirest.post(SERVER_URL+'loadFile', headers={ "Content-Type": "application/json" }, params=json.dumps({"command": "LOAD_FILE testfile"}))
        tmp_res = json.loads(res.raw_body)
        sucess=sucess&(tmp_res['success']==True)
        sucess=sucess&(tmp_res['data']=="FILE_FOUND_AND_LOADED")
        res=unirest.get(SERVER_URL+'killProcess', headers={}, params={})
        res=unirest.post(SERVER_URL+'loadFile', headers={ "Content-Type": "application/json" }, params=json.dumps({"command": "LOAD_FILE testfile"}))
        res=unirest.post(SERVER_URL+'loadFile', headers={ "Content-Type": "application/json" }, params=json.dumps({"command": "LOAD_FILE testfil"}))
        tmp_res = json.loads(res.raw_body)
        sucess=sucess&(tmp_res['success']==False)
        sucess=sucess&(tmp_res['data']=="FILE_NOT_FOUND")
        res=unirest.get(SERVER_URL+'killProcess', headers={}, params={})
        self.assertEqual(sucess, 1)

    def test_S_FILE_CHOSEN_R_PLAY_FILE(self):
        sucess=1
        res=unirest.post(SERVER_URL+'loadFile', headers={ "Content-Type": "application/json" }, params=json.dumps({"command": "LOAD_FILE testfile"}))
        res=unirest.get(SERVER_URL+'launchSynchronization', headers={}, params={})
        tmp_res = json.loads(res.raw_body)
        sucess=sucess&(tmp_res['success']==True)
        sucess=sucess&(tmp_res['data']=="SYNCHRONIZATION_LAUNCHED")
        res=unirest.get(SERVER_URL+'killProcess', headers={}, params={})
        self.assertEqual(sucess, 1)

    def test_S_FILE_CHOSEN_R_GET_TIMST(self):
        sucess=1
        res=unirest.post(SERVER_URL+'loadFile', headers={ "Content-Type": "application/json" }, params=json.dumps({"command": "LOAD_FILE testfile"}))
        res=unirest.get(SERVER_URL+'getTimeStamp', headers={}, params={})
        tmp_res = json.loads(res.raw_body)
        sucess=sucess&(tmp_res['success']==True)
        res=unirest.get(SERVER_URL+'killProcess', headers={}, params={})
        self.assertEqual(sucess, 1)

    # State : SYNC_STARTED

    def test_S_SYNC_STARTED_R_LOAD_FILE(self):
        sucess=1
        res=unirest.post(SERVER_URL+'loadFile', headers={ "Content-Type": "application/json" }, params=json.dumps({"command": "LOAD_FILE testfile"}))
        res=unirest.get(SERVER_URL+'launchSynchronization', headers={}, params={})
        res=unirest.post(SERVER_URL+'loadFile', headers={ "Content-Type": "application/json" }, params=json.dumps({"command": "LOAD_FILE testfile"}))
        tmp_res = json.loads(res.raw_body)
        sucess=sucess&(tmp_res['success']==True)
        sucess=sucess&(tmp_res['data']=="FILE_FOUND_AND_LOADED")
        res=unirest.get(SERVER_URL+'killProcess', headers={}, params={})
        self.assertEqual(sucess, 1)

    def test_S_SYNC_STARTED_R_PLAY_FILE(self):
        sucess=1
        res=unirest.post(SERVER_URL+'loadFile', headers={ "Content-Type": "application/json" }, params=json.dumps({"command": "LOAD_FILE testfile"}))
        res=unirest.get(SERVER_URL+'launchSynchronization', headers={}, params={})
        res=unirest.get(SERVER_URL+'launchSynchronization', headers={}, params={})
        tmp_res = json.loads(res.raw_body)
        sucess=sucess&(tmp_res['success']==False)
        sucess=sucess&(tmp_res['data']=="THIS_REQUEST_CANNOT_BE_HANDLED_AS_FILE_IS_PLAYING")
        res=unirest.get(SERVER_URL+'killProcess', headers={}, params={})
        self.assertEqual(sucess, 1)

    def test_S_SYNC_STARTED_R_GET_TIMST(self):
        sucess=1
        res=unirest.post(SERVER_URL+'loadFile', headers={ "Content-Type": "application/json" }, params=json.dumps({"command": "LOAD_FILE testfile"}))
        res=unirest.get(SERVER_URL+'launchSynchronization', headers={}, params={})
        res=unirest.get(SERVER_URL+'getTimeStamp', headers={}, params={})
        tmp_res = json.loads(res.raw_body)
        sucess=sucess&(tmp_res['success']==True)
        res=unirest.get(SERVER_URL+'killProcess', headers={}, params={})
        self.assertEqual(sucess, 1)

if __name__ == '__main__':
    suite = unittest.TestSuite()
    suite.addTest(TestStringMethods('test_S_SYNC_ENDED_R_LOAD_FILE'))

    #suite.addTest(TestStringMethods('test_S_FILE_CHOSEN_R_LOAD_FILE'))
    #suite.addTest(TestStringMethods('test_S_FILE_CHOSEN_R_PLAY_FILE'))
    #suite.addTest(TestStringMethods('test_S_FILE_CHOSEN_R_GET_TIMST'))

    #suite.addTest(TestStringMethods('test_S_SYNC_STARTED_R_LOAD_FILE'))
    #suite.addTest(TestStringMethods('test_S_SYNC_STARTED_R_PLAY_FILE'))
    #suite.addTest(TestStringMethods('test_S_SYNC_STARTED_R_GET_TIMST'))

    unittest.TextTestRunner(verbosity=2).run(suite)