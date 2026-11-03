import pandas as pd
import matplotlib.pyplot as plt

# Baca CSV dari STM32
df = pd.read_csv('geolistrik_stm32.csv') # AB/2,V,I,Rho
plt.figure()
plt.loglog(df['AB/2'], df['Rho'], 'o-')
plt.xlabel('AB/2 (m)')
plt.ylabel('Apparent Resistivity (Ohm.m)')
plt.title('Geolistrik 10 Data - Central Java')
plt.grid(True)
plt.savefig('graph_10data.png')
print(df)
