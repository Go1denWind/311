%% Current-sense amplification: input (I_FB, C2) vs output (I_FBF, C1)
% One figure per case, input and output on the same axes, over three 10 us
% switching periods. Each CSV: line 1 = text header, then time [s], voltage [V].
% Input files are channel C2, output files are channel C1 (same time base).
clear; clc; close all;

addpath('C:\Users\tytom\Documents\GitHub\311\Oscilloscope\Output Generator\Open Loop Testing')

Ts   = 10e-6;               % switching period [s]
Twin = 3*Ts;                % three periods = 30 us
t0   = -20e-6;              % window start [s]: a switching edge (edges fall every 10 us at ..., -20, -10, 0, ...)

% One row per case: input file, output file, load, input voltage
cs = struct( ...
    'fileI', {'5_25_I.CSV', '5_35_I.CSV', '50_25_I2.CSV', '50_35_I.CSV'}, ...
    'fileO', {'5_25_O.CSV', '5_35_O.CSV', '50_25_O.CSV',  '50_35_O.CSV'}, ...
    'Rl',    {5,    5,    50,   50 }, ...      % load [ohm]
    'Vin',   {2.5,  3.5,  2.5,  3.5});         % input voltage [V]

for k = 1:numel(cs)
    TI = readtable(cs(k).fileI, 'HeaderLines', 1, 'ReadVariableNames', false);
    TO = readtable(cs(k).fileO, 'HeaderLines', 1, 'ReadVariableNames', false);
    tI = TI{:,1};  vI = TI{:,2}*1e3;     % input,  mV
    tO = TO{:,1};  vO = TO{:,2}*1e3;     % output, mV

    selI = tI >= t0 & tI <= t0 + Twin;
    selO = tO >= t0 & tO <= t0 + Twin;

    % Mean values over the whole capture (6 complete periods) and the gain
    mI = mean(vI);
    mO = mean(vO);

    figure('Name', sprintf('%gohm_%.1fV', cs(k).Rl, cs(k).Vin), ...
           'Color', 'w', 'Position', [100 100 900 450]);
    plot((tI(selI) - t0)*1e6, vI(selI), 'LineWidth', 1); hold on;
    plot((tO(selO) - t0)*1e6, vO(selO), 'LineWidth', 1);
    grid on;
    xlim([0 Twin*1e6]);
    xticks(0:Ts*1e6:Twin*1e6);           % one tick per switching period
    xlabel('Time (\mus)');
    ylabel('Current sense signal (mV)');
    title(sprintf('Current sense - %g\\Omega load, %.1fV input', cs(k).Rl, cs(k).Vin));
    legend('I_{FB} (input)', 'I_{FBF} (output)', 'Location', 'northeast');
    text(0.03, 0.92, sprintf('I_{FB} mean = %.1f mV\nI_{FBF} mean = %.1f mV\nMean gain = %.2fx', ...
         mI, mO, mO/mI), 'Units', 'normalized', 'FontSize', 9, ...
         'VerticalAlignment', 'top', 'BackgroundColor', 'w', 'EdgeColor', [0.7 0.7 0.7]);

    fprintf('%g ohm, %.1f V: I_FB mean = %.2f mV, I_FBF mean = %.2f mV, gain = %.2fx\n', ...
            cs(k).Rl, cs(k).Vin, mI, mO, mO/mI);
end

%% ---- Voltage divider check: divider input (C2) vs divider output (C1) ----
% Same time base and window as above. Values are in volts.
TI = readtable('VDIV_I.CSV', 'HeaderLines', 1, 'ReadVariableNames', false);
TO = readtable('VDIV_O.CSV', 'HeaderLines', 1, 'ReadVariableNames', false);
tI = TI{:,1};  vI = TI{:,2};             % divider input,  V
tO = TO{:,1};  vO = TO{:,2};             % divider output, V

selI = tI >= t0 & tI <= t0 + Twin;
selO = tO >= t0 & tO <= t0 + Twin;

mI = mean(vI);
mO = mean(vO);

figure('Name', 'Voltage divider', 'Color', 'w', 'Position', [100 100 900 450]);
plot((tI(selI) - t0)*1e6, vI(selI), 'LineWidth', 1); hold on;
plot((tO(selO) - t0)*1e6, vO(selO), 'LineWidth', 1);
grid on;
xlim([0 Twin*1e6]);
xticks(0:Ts*1e6:Twin*1e6);
xlabel('Time (\mus)');
ylabel('Voltage (V)');
title('Voltage divider check');
legend('Divider input (C2)', 'Divider output (C1)', 'Location', 'east');
text(0.03, 0.92, sprintf('Input mean = %.2f V\nOutput mean = %.2f V\nOutput / input = %.3f', ...
     mI, mO, mO/mI), 'Units', 'normalized', 'FontSize', 9, ...
     'VerticalAlignment', 'top', 'BackgroundColor', 'w', 'EdgeColor', [0.7 0.7 0.7]);

fprintf('Divider: input mean = %.3f V, output mean = %.3f V, ratio = %.3f\n', mI, mO, mO/mI);