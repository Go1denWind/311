%% Plot WFM04/06/07/08 over three switching periods (Ts = 10 us)
% CSV format: line 1 = text header ("in s,C1 in V"), then two numeric
% columns: time [s], C1 voltage [V]. 75001 rows, dt = 0.8 ns, -30 us to +30 us.
clear; clc; close all;

addpath('C:\Users\tytom\Documents\GitHub\311\Oscilloscope\Output Generator\Open Loop Testing')

Ts      = 10e-6;            % switching period [s]
nPeriod = 3;                % number of periods to show
Twin    = nPeriod*Ts;       % 30 us window

files = {'WFM04.CSV','WFM06.CSV','WFM07.CSV','WFM08.CSV'};
info  = {'WFM04 - 35% duty cycle, 5\Omega load, 2.5V_{in}', ...
    'WFM06 - 47% duty cycle, 50\Omega load, 2.5V_{in}', ...
    'WFM07 - 67% duty cycle, 50\Omega load, 3.5V_{in}', ...
    'WFM08 - 60% duty cycle, 5\Omega load, 3.5V_{in}'};

for k = 1:numel(files)
    % ---- Load: skip the header row, read the two numeric columns ----
    T = readtable(files{k}, 'HeaderLines', 1, 'ReadVariableNames', false);
    t = T{:,1};             % time [s]
    v = T{:,2};             % voltage [V]

    % ---- Window: start at the first large switching spike ----
    % (spike = more than 1.3 V from the median level; spikes are 10 us apart)
    idx = find(abs(v - median(v)) > 1.3, 1, 'first');
    t0  = t(idx);
    if t0 + Twin > t(end)   % safety: keep the window inside the data
        t0 = t(end) - Twin;
    end
    sel = (t >= t0) & (t <= t0 + Twin);

    % ---- Plot ----
    figure('Name', files{k}, 'Color', 'w', 'Position', [100 100 900 450]);
    plot((t(sel) - t0)*1e6, v(sel), 'LineWidth', 1);
    grid on;
    xlim([0 Twin*1e6]);
    xticks(0:Ts*1e6:Twin*1e6);      % one tick per switching period
    xlabel('Time (\mus)');
    ylabel('C1 Voltage (V)');
    title(info{k});
end
