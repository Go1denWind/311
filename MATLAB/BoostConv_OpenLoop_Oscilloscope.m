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

%% ---- WFM11 and WFM09: OVP response (single edge, full capture) ----
% These captures are 960 ns long (-480 ns to +480 ns), so each is shown in
% full rather than over three switching periods. (WFM11: dt = 0.4 ns,
% WFM09: dt = 0.8 ns.)
ovpFiles  = {'WFM11.CSV', 'WFM09.CSV'};
ovpTitles = {'WFM11 - OVP response', 'WFM09 - OVP response'};

for k = 1:numel(ovpFiles)
    T = readtable(ovpFiles{k}, 'HeaderLines', 1, 'ReadVariableNames', false);
    t = T{:,1};
    v = T{:,2};

    % Light smoothing (~3.6 ns) so threshold crossings aren't triggered by noise
    dt = mean(diff(t));
    vs = movmean(v, max(1, round(3.6e-9/dt)));

    vLo = median(v(t < -200e-9));       % low level before the edge
    vHi = median(v(t >  300e-9));       % settled high level
    lvl = @(f) vLo + f*(vHi - vLo);

    % Linear-interpolated time at which the smoothed signal first crosses L
    crossT = @(L) interpCross(t, vs, L);
    t10 = crossT(lvl(0.1));
    t50 = crossT(lvl(0.5));
    t90 = crossT(lvl(0.9));

    figure('Name', ovpFiles{k}, 'Color', 'w', 'Position', [100 100 900 450]);
    plot(t*1e9, v, 'LineWidth', 1); hold on;
    plot([t10 t90]*1e9, [lvl(0.1) lvl(0.9)], 'ro', 'MarkerFaceColor', 'r');
    yline(vHi, 'k--');
    grid on;
    xlabel('Time (ns)');
    ylabel('C1 Voltage (V)');
    title(ovpTitles{k});
    legend('C1', '10% / 90% points', sprintf('Final level = %.2f V', vHi), ...
           'Location', 'southeast');
    text(0.03, 0.92, sprintf('10-90%% rise time = %.0f ns', (t90 - t10)*1e9), ...
         'Units', 'normalized', 'FontSize', 11);
    fprintf('%s: 10-90%% rise time = %.1f ns, 50%% point at t = %.1f ns\n', ...
            ovpFiles{k}, (t90 - t10)*1e9, t50*1e9);
end


%% ---- WFM12: comparator / OVP test, rise time of the output ramp ----
% Long capture (-1.84 ms to +10.16 ms, dt = 92 ns). The sudden drop to 0 V
% (and restart) is ignored; the rise time is measured on the first ramp, from
% 10% to 90% of the level reached just before the drop.
T12 = readtable('WFM12.CSV', 'HeaderLines', 1, 'ReadVariableNames', false);
t12 = T12{:,1};
v12 = T12{:,2};
vs12 = movmean(v12, 21);                % ~2 us smoothing for threshold crossings

% Locate the drop only to know where the ramp ends (steepest falling edge)
dv = diff(vs12);
dv([1:15, end-14:end]) = 0;
[~, iFall] = min(dv);
tFall = t12(iFall);

vLo = median(v12(t12 < -1.0e-3));                          % level before the ramp
vHi = median(v12(t12 > tFall-20e-6 & t12 < tFall-5e-6));   % level reached by the ramp
lvl = @(f) vLo + f*(vHi - vLo);

ramp = t12 < tFall - 5e-6;              % first ramp only
tr = t12(ramp);  yr = vs12(ramp);
t10 = interpCross(tr, yr, lvl(0.1));
t90 = interpCross(tr, yr, lvl(0.9));

figure('Name', 'WFM12', 'Color', 'w', 'Position', [100 100 900 450]);
plot(t12*1e3, v12, 'LineWidth', 0.8); hold on;
plot([t10 t90]*1e3, [lvl(0.1) lvl(0.9)], 'ro', 'MarkerFaceColor', 'r');
grid on;
xlabel('Time (ms)');
ylabel('C1 Voltage (V)');
title('WFM12 - Comparator response to overvoltage');
legend('C1', '10% / 90% points', 'Location', 'southeast');
text(0.03, 0.92, sprintf('10-90%% rise time = %.2f ms', (t90 - t10)*1e3), ...
     'Units', 'normalized', 'FontSize', 11);
fprintf('WFM12: 10-90%% rise time = %.2f ms\n', (t90 - t10)*1e3);

%% ---- WFM13 / WFM14: output voltage with 6.0 V (no OVP) and 6.1 V (OVP) input ----
% Long captures (-2.6 ms to +9.4 ms, dt = 92 ns) with no switching edges to
% align to, so the window is the first three 10 us periods from t = 0.
ovFiles  = {'WFM13.CSV', 'WFM14.CSV'};
ovTitles = {'WFM13 - V_{in} = 6.0 V (OVP not active)', ...
            'WFM14 - V_{in} = 6.1 V (OVP active)'};

for k = 1:numel(ovFiles)
    T = readtable(ovFiles{k}, 'HeaderLines', 1, 'ReadVariableNames', false);
    t = T{:,1};
    v = T{:,2};

    sel = t >= 0 & t <= Twin;           % three 10 us periods starting at t = 0

    figure('Name', ovFiles{k}, 'Color', 'w', 'Position', [100 100 900 450]);
    plot(t(sel)*1e6, v(sel), 'LineWidth', 1);
    grid on;
    xlim([0 Twin*1e6]);
    xticks(0:Ts*1e6:Twin*1e6);
    ylim([-1 11]);                      % same y-range on both so they compare directly
    xlabel('Time (\mus)');
    ylabel('Output Voltage (V)');
    title(ovTitles{k});
    text(0.03, 0.92, sprintf('Mean V_{out} = %.2f V', mean(v(sel))), ...
         'Units', 'normalized', 'FontSize', 11);
end

%% ---- Current sense: input (I_FB, C2) and output (I_FBF, C1) on the same axes ----
% Each case has an input file (C2) and an output file (C1) captured on the same
% time base. Add more rows here for the other Rl / Vin / duty combinations.
cs = struct( ...
    'fileI', {'5_25_I.CSV'}, ...   % input  (I_FB)
    'fileO', {'5_25_O.CSV'}, ...   % output (I_FBF)
    'Rl',    {5}, ...              % load [ohm]
    'Vin',   {2.5}, ...            % input voltage [V]
    'D',     {0.5});               % duty cycle

for k = 1:numel(cs)
    TI = readtable(cs(k).fileI, 'HeaderLines', 1, 'ReadVariableNames', false);
    TO = readtable(cs(k).fileO, 'HeaderLines', 1, 'ReadVariableNames', false);
    tI = TI{:,1};  vI = TI{:,2}*1e3;    % mV
    tO = TO{:,1};  vO = TO{:,2}*1e3;    % mV

    % Start the window on a rising edge of the output pulse (30% of smoothed range)
    vs  = movmean(vO, 100);
    thr = min(vs) + 0.3*(max(vs) - min(vs));
    ie  = find(vs(1:end-1) < thr & vs(2:end) >= thr);
    ie  = ie(tO(ie) + Twin <= tO(end));          % keep only edges with a full window after them
    t0  = tO(ie(1));

    selI = tI >= t0 & tI <= t0 + Twin;
    selO = tO >= t0 & tO <= t0 + Twin;

    % Means over the whole capture (an integer number of periods)
    mI = mean(vI);  mO = mean(vO);

    figure('Name', 'Current sense', 'Color', 'w', 'Position', [100 100 900 450]);
    plot((tI(selI) - t0)*1e6, vI(selI), 'LineWidth', 1); hold on;
    plot((tO(selO) - t0)*1e6, vO(selO), 'LineWidth', 1);
    grid on;
    xlim([0 Twin*1e6]);
    xticks(0:Ts*1e6:Twin*1e6);
    xlabel('Time (\mus)');
    ylabel('Current sense signal (mV)');
    title(sprintf('Current sense - R_L = %g\\Omega, V_{in} = %.1fV, D = %.1f', ...
          cs(k).Rl, cs(k).Vin, cs(k).D));
    legend('I_{FB} (input)', 'I_{FBF} (output)', 'Location', 'northeast');
    text(0.03, 0.92, sprintf('I_{FB} mean = %.1f mV\nI_{FBF} mean = %.1f mV\nMean gain = %.2fx', ...
         mI, mO, mO/mI), 'Units', 'normalized', 'FontSize', 9, ...
         'VerticalAlignment', 'top', 'BackgroundColor', 'w', 'EdgeColor', [0.7 0.7 0.7]);
end

%% Local functions (must stay at the end of the script; need R2016b or newer)
function tc = interpCross(t, y, L)
    i  = find(y >= L, 1);               % first sample at/above level L (rising)
    tc = t(i-1) + (L - y(i-1)) / (y(i) - y(i-1)) * (t(i) - t(i-1));
end