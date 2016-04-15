import cv2
import sys
import time
from RPIO import PWM

servo = PWM.Servo()

#Send signals through pin 18 which corresponds to physical pin 12
pin = 18

cascPath = sys.argv[1]
faceCascade = cv2.CascadeClassifier(cascPath)

#Open cv's video capture function
video_capture = cv2.VideoCapture(0)

#The duty cycle has been set such that in the beginning the motor is set at 0 degrees.
currdc = 1540
servo.set_servo(pin, currdc)

while True:
    # Capture frame-by-frame
    ret, frame = video_capture.read()

    #Convert frame to grayscale
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    #print(video_capture.get(3))
    #print(video_capture.get(4))

    #Detect faces in the frame
    faces = faceCascade.detectMultiScale(
        gray,
        scaleFactor=1.4,
        minNeighbors=3,
        minSize=(80, 80),
        flags=cv2.cv.CV_HAAR_SCALE_IMAGE
    )
    #print("Capture frame 4")

    # Draw a rectangle around the faces
    minx = 5000
    maxx = 0    
    prevcenter = 0
    for (x, y, w, h) in faces:
        #cv2.rectangle(frame, (x, y), (x+w, y+h), (0, 255, 0), 2)
        sys.stdout.write("\nCoordinates of the face center:- ("+str(x+w/2)+", "+str(y+h/2)+")\n")
        #print(x+w/2, y+h/2)
        center = x+w/2
        #If the difference between face centers isn't too much, do nothing (to avoid jitter)
        if abs(prevcenter - center) <= 50:
            continue
        dist = center - 320.0

        #Find angle by which to rotate the device
        angle = dist*53.0/640.0
        
        sys.stdout.write("Angle to be rotated:- "+str(angle)+"\n\n")
        offset = angle*2020.0/225.0
        currdc = currdc + offset
        currdc = int(currdc)
        currdc = currdc - currdc%10
        if currdc > 2550:
            currdc = 2550
        if currdc < 520:
            currdc = 550

        #print("pulse: ",currdc)
        #Release capture before sending signal to motor
        video_capture.release()
        servo.set_servo(pin, currdc)

        #Start the capture again
        video_capture = cv2.VideoCapture(0)      
        prevcenter = center
        
    # Display the resulting frame
    #cv2.imshow('Video', frame)

    cv2.waitKey(1)
    if 0xFF == ord('q'):
        break

# When everything is done, release the capture
video_capture.release()
cv2.destroyAllWindows()

