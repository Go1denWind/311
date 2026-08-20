RL = 50;
Rp = 5*10^-3;
C = 330*10^-6;

num = [Rp*C, 1];
den = [C*(Rp+RL), 1];

Tsys = tf(num, den);

Tsys = RL*Tsys

bode(Tsys)

pzplot(Tsys)
