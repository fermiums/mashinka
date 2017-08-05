# Модуль socket для сетевого программирования
from socket import *
import pygame
import subprocess
import time

# данные сервера
host = '10.0.1.132'#Stream
#host = '169.254.252.40'#LAN
port = 2323
addr = (host, port)

# socket - функция создания сокета
# первый параметр socket_family может быть AF_INET или AF_UNIX
# второй параметр socket_type может быть SOCK_STREAM(для TCP) или SOCK_DGRAM(для UDP)
udp_socket = socket(AF_INET, SOCK_DGRAM)
# bind - связывает адрес и порт с сокетом
udp_socket.bind(addr)

print('жду машинку...')

# recvfrom - получает UDP сообщения
conn, addr = udp_socket.recvfrom(64)
print('client addr: ', addr)





##############======Запускаем джойстик в бесшумном режиме=============
password = 'volodya89'
cmd = 'echo ' + password+ ' | sudo -S xboxdrv --chatpad --silent'
#sudo xboxdrv --silent --detach-kernel-driver

PIPE = subprocess.PIPE
p = subprocess.Popen(cmd, shell=True, stdin=PIPE, stdout=PIPE,
        stderr=subprocess.STDOUT, close_fds=True)


#while True:
#    s = p.stdout.readline()
#    if s==b'X1:   128 Y1:  -128  X2:   128 Y2:  -128  du:0 dd:0 dl:0 dr:0  back:0 guide:0 start:0  TL:0 TR:0  A:0 B:0 X:0 Y:0  LB:0 RB:0  LT:  0 RT:  0\n': break
#    print (s)

##############===========================================================
#print ('Совпало')



# Define some colors
BLACK = (0, 0, 0)
WHITE = (255, 255, 255)


# This is a simple class that will help us print to the screen
# It has nothing to do with the joysticks, just outputting the
# information.
class TextPrint:
    def __init__(self):
        self.reset()
        self.font = pygame.font.Font(None, 20)

    def print(self, screen, textString):
        textBitmap = self.font.render(textString, True, BLACK)
        screen.blit(textBitmap, [self.x, self.y])
        self.y += self.line_height

    def reset(self):
        self.x = 10
        self.y = 10
        self.line_height = 15

    def indent(self):
        self.x += 10

    def unindent(self):
        self.x -= 10


pygame.init()

# Set the width and height of the screen [width,height]
size = [500, 700]
screen = pygame.display.set_mode(size)

pygame.display.set_caption("My Game")

# Loop until the user clicks the close button.
done = False

# Used to manage how fast the screen updates
clock = pygame.time.Clock()

# Initialize the joysticks
pygame.joystick.init()

# Get ready to print
textPrint = TextPrint()

#for raspberry
data=bytes("", encoding = 'utf-8')

        # DRAWING STEP
        # First, clear the screen to white. Don't put other drawing commands
        # above this, or they will be erased with this command.
screen.fill(WHITE)
textPrint.reset()

            # Get count of joysticks
joystick_count = pygame.joystick.get_count()

textPrint.print(screen, "Number of joysticks: {}".format(joystick_count))
textPrint.indent()

            # For each joystick:
for i in range(joystick_count):
    joystick = pygame.joystick.Joystick(i)
    joystick.init()

textPrint.print(screen, "Joystick {}".format(i))
textPrint.indent()

                # Get the name from the OS for the controller/joystick
name = joystick.get_name()
textPrint.print(screen, "Joystick name: {}".format(name))




# -------- Main Program Loop -----------
while done == False:
    # EVENT PROCESSING STEP
    for event in pygame.event.get():  # User did something
        if event.type == pygame.QUIT:  # If user clicked close
            done = True  # Flag that we are done so we exit this loop

#        # Possible joystick actions: JOYAXISMOTION JOYBALLMOTION JOYBUTTONDOWN JOYBUTTONUP JOYHATMOTION
#        if event.type == pygame.JOYBUTTONDOWN:
#            print("Joystick button pressed.")
#        if event.type == pygame.JOYBUTTONUP:
#            print("Joystick button released.")






        # Usually axis run in pairs, up/down for one, and left/right for
        # the other.
    axes = joystick.get_numaxes()
#        textPrint.print(screen, "Number of axes: {}".format(axes))
#        textPrint.indent()

    for i in range(axes):
        axis = joystick.get_axis(i)
#            textPrint.print(screen, "Axis {} value: {:>6.3f}".format(i, axis))
            # sendto - передача сообщения UDP
        data+=bytes("{}A{:>1.2f} ".format(i, axis), encoding = 'utf-8')
#        textPrint.unindent()

    buttons = joystick.get_numbuttons()
#        textPrint.print(screen, "Number of buttons: {}".format(buttons))
#        textPrint.indent()

    for i in range(buttons):
        button = joystick.get_button(i)
#            textPrint.print(screen, "Button {:>2} value: {}".format(i, button))
            # sendto - передача сообщения UDP
        data+=bytes("{}B{:>1}".format(i, button), encoding = 'utf-8')
#        textPrint.unindent()

      
    

        # Hat switch. All or nothing for direction, not like joysticks.
        # Value comes back in an array.
#        hats = joystick.get_numhats()
#        textPrint.print(screen, "Number of hats: {}".format(hats))
#        textPrint.indent()

#        for i in range(hats):
#            hat = joystick.get_hat(i)
#            textPrint.print(screen, "Hat {} value: {}".format(i, str(hat)))
#        textPrint.unindent()

#        textPrint.unindent()

    udp_socket.sendto(data, addr)
    data=b''   
    # ALL CODE TO DRAW SHOULD GO ABOVE THIS COMMENT

    # Go ahead and update the screen with what we've drawn.
    pygame.display.flip()

    # Limit to 20 frames per second
    #clock.tick(20)

    time.sleep(0.1)

    


# Close the window and quit.
# If you forget this line, the program will 'hang'
# on exit if running from IDLE.
pygame.quit()
