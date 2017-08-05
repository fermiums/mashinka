import time
#import SimpleCV

#GPIO initialization:
#GPIO.setmode(GPIO.BCM) #Pinout map
#GPIO.setwarnings(False) #Warnings off
#GPIO.setup(23, GPIO.OUT) #23 pin - output

#time.sleep(1)  #second


#na = 'test_foto'
#me = '.jpg'
#name = na + me

#Camera initialization:
#camera = PiCamera()
#camera.rotation = 180
#camera.resolution = (320, 240)

#GPIO.output(23, GPIO.LOW) #LED On
#camera.start_preview()
#camera.capture(name)
#camera.stop_preview()
#GPIO.output(23, GPIO.HIGH) #LED Off

#GPIO.cleanup() #GPIO CLEANUP
print('_RASBERI OK_')
time.sleep(1)  #second
kolvo_errore=0
time.sleep(1)  #second
         
d=0     
while 1:
     d+=1
     print("stroka12345___%0.2d"%d)
     if d==99:
          d=0
