fGravity = -9.8
fWater = 9.0
time = [0:0.1:30]
depth = (0.5*(fGravity+fWater))*(time.^2); 
%displacement = v(i)*t+0.5*a*(t^2)
%v(f)^2 = v(i)^2+(2*a*d)
%v(f) = v(i) + a*t
%d =[(v(i)+v(f))/2]*t

figure
hold on
plot(time, depth, 'b.-', 'LineWidth', 1, 'MarkerSize', 20);
title('Depth v Time')
xlabel('time(seconds)') % x-axis label
ylabel('Depth (meters)') % y-axis label
%cftool
