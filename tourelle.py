import struct
import smbus

TOURELLE_I2C_ADDR = 69


class Tourelle(object):
   '''Implemente la communication avec la tourelle de detection  de 2012
   bus I2C du robot.'''
   def __init__(self, addr=TOURELLE_I2C_ADDR, bus=1):
      super(Tourelle, self).__init__()
      if isinstance(bus, smbus.SMBus):
         self.bus = bus
      else:
         self.bus = smbus.SMBus(bus)
      self.addr=addr
      self._moteur_dead=False #le moteur est t'il mort ?
      self._points=None
      
   def update(self):
      try:
         ret = self.bus.read_i2c_block_data(self.addr,ord('E'),1)
         #print "etat get", ret[0]
         if ret[0] == 66:
            self._moteur_dead=True
         elif ret[0] != 0:
            print "Erreur tourelle ", ret[0]
         else:
            self._moteur_dead=False
         #print "try get position"
         ret = self.bus.read_i2c_block_data(self.addr, ord('D'), 18)
         ret = ''.join([chr(i) for i in ret])
         self._points = struct.unpack('<BHBHBHBHBHBH', ret)
         #print "success !"
         if len(ret) < 18:
            print "Erreur i2c tourelle mauvaise taille du message !"
            
      except IOError as e:
         print "I2C error update tourelle"
         raise e
   
   def is_moteur_dead(self):
      return self._moteur_dead
      
   def robot_derriere(self):
      if self._points is None:
         print "Erreur oublie de l'appel a 'update'"
         return True
         
      for i in range(6):
         if self._points[i*2] > 0:
            angle = self._points[i*2+1]
            if angle > 500 or angle < 2900:
               return True
      return False
      
   def robot_devant(self):
      if self._points is None:
         print "Erreur oublie de l'appel a 'update'"
         return True
         
      for i in range(6):
         if self._points[i*2] > 0:
            angle = self._points[i*2+1]
            if angle < 1830 and angle > 1000:
               return True
      return False 
      
   def robot_a_droite(self):
      if self._points is None:
         print "Erreur oublie de l'appel a 'update'"
         return True
         
      for i in range(6):
         if self._points[i*2] > 0:
            angle = self._points[i*2+1]
            if angle > 200 and angle < 1350:
               return True
      return False
      
   def robot_a_gauche(self):
      if self._points is None:
         print "Erreur oublie de l'appel a 'update'"
         return True
         
      for i in range(6):
         if self._points[i*2] > 0:
            angle = self._points[i*2+1]
            if angle > 2000 and angle < 3200:
               return True
      return False
      
   def get_points(self):
      return self._points
      
   def position(self):
        ret = self.bus.read_i2c_block_data(self.addr, ord('D'), 18)
        ret = ''.join([chr(i) for i in ret])
        d1, a1, d2, a2, d3, a3, d4, a4, d5, a5, d6, a6 = struct.unpack('<BHBHBHBHBHBH', ret)
        return d1, a1, d2, a2, d3, a3, d4, a4, d5, a5, d6, a6   