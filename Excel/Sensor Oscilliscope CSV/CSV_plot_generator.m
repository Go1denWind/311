close all;
clear all:
clc;

function plot_data(file_name)

    % CSV file name
    % file_name = "U10_power_switch_U6_held_closed";
    
    % create matrix from CSV file
    data_matrix = readmatrix(file_name + '.csv');
    
    % Create column array, does not include time column
    columns = [];
    
    % Get number of columns
    num_col = size(data_matrix, 2);
    
    % Extract columns using matrix indexing (Row, Column)
    % Time axis is always column 1
    
    for i = 1:num_col
        x = data_matrix(:, i);
        columns = [columns, x];
    end
    
    % Plot the data
    figure;
    hold on;
    
    for i = 2:(num_col)
        plot(columns(:, 1), columns(:, i));
        
    end
    
    hold off;
    xlabel('Time, [s]');
    ylabel('Voltage, [V]');

end

plot_data("U6_power_switch");
title("U6 Output Pin Voltage for a Reactangular ON Pin Voltage");
grid on;

plot_data("U10_power_switch_U6_held_closed");
title("U10 Output Pin Voltage for a Reactangular ON Pin Voltage, U6 Held ON");
grid on;

plot_data("touch_sensor_no_touch");
xlim([-10e-6, 10e-6]);
title("Touch Sensor Oscillator Output Voltage, No Touch");
grid on;

plot_data("touch_sensor_touched");
xlim([-10e-6, 10e-6]);
title("Touch Sensor Oscillator Output Voltage, Touch");
grid on;

plot_data("current_sensor_0.255A_input");
xlim([0, 0.06]);
ylim([0, 5]);
title("Current Sensor Output Voltage, Charging 0.255 A");
grid on;

% v2 has R29 = 1.2 kohm which is what is on the altium schematic.
plot_data("voltage_sensor_ramp_input_v2");
xlim([-0.6, -0.25]);
yticks(0:0.5:6);
title("Voltage Sensor Output for a Ramp Input");
legend("Input", "Output");
grid on;

plot_data("window_comparator_current");
xlim([0, 0.012]);
title("Current Sensor Window Comparator Output Voltage for a Sinusoidal Input");
grid on;

plot_data("window_comparator_voltage");
xlim([0, 0.012]);
title("Voltage Sensor Window Comparator Output Voltage for a Sinusoidal Input");
grid on;

plot_data("window_comparator_temperature");
xlim([0, 0.012]);
title("Temperature Sensor Window Comparator Output Voltage for a Sinusoidal Input");
grid on;