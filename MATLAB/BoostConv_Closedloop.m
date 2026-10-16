%% Closed-loop boost converter test plots (5 V out)
clear; clc; close all;

dataDir = "C:\Users\tytom\Documents\GitHub\311\Oscilloscope\Output Generator\Closed Loop Testing";

loadLabels = {'5 \Omega', '10 \Omega', '50 \Omega'};     % columns of each grid

%% Window 1: 2.5 V input (rows = DC / ripple / rise, columns = 5 / 10 / 50 ohm)
files25 = { '2V5_5_DC.CSV',  '2V510DC.CSV',  '2V550DC.CSV';
            '2V55AC.CSV',    '2V510AC.CSV',  '2V550AC.CSV';
            '2V5_5_TR.CSV',  '2V510TR.CSV',  '2V550TR.CSV' };
res25 = buildGridFigure(dataDir, files25, loadLabels, '2.5 V', ...
                        'Closed-Loop Testing - 2.5 V input', ...
                        fullfile(dataDir, 'closed_loop_results_2V5.png'));

%% Window 2: 3.5 V input
files35 = { '3V55DC.CSV',  '3V510DC.CSV',  '3V550DC.CSV';
            '3V55AC.CSV',  '3V510AC.CSV',  '3V550AC.CSV';
            '3V55TR.CSV',  '3V510TR.CSV',  '3V550TR.CSV' };
res35 = buildGridFigure(dataDir, files35, loadLabels, '3.5 V', ...
                        'Closed-Loop Testing - 3.5 V input', ...
                        fullfile(dataDir, 'closed_loop_results_3V5.png'));

%% Window 3: slope compensation (2.5 V in, 5 ohm) -----------------------
SC  = readmatrix(fullfile(dataDir, '2V5_5_SC.CSV'), 'NumHeaderLines', 1);
tSC = SC(:,1)*1e6;                   % us
vSC = SC(:,2);

% Switching frequency from falling edges (debounced so the mid-ramp glitch is ignored)
vSm  = movmean(vSC, 20);
idx  = find(vSm(1:end-1) > 0.4 & vSm(2:end) <= 0.4);
keep = idx(1);
for k = idx(2:end).'
    if tSC(k) - tSC(keep(end)) > 25      % us
        keep(end+1) = k; %#ok<SAGROW>
    end
end
Tsw = median(diff(tSC(keep)));           % us
fsw = 1e6/Tsw;                           % Hz

% Ramp start/end levels while the signal is high (5th-95th percentile)
[rLo, rHi] = bandEdges(vSC(vSC > 0.5), 5);

zoomLim = [-30 90];                      % us; use [min(tSC) max(tSC)] for the full capture

figSC = figure('Name','Slope Compensation','Color','w','Position',[150 100 1000 450]);
plot(tSC, vSC, 'Color', [0 0.447 0.741], 'LineWidth', 0.8); hold on;
yline(rLo, ':', sprintf('%.2f V', rLo), 'Color',[0.85 0.325 0.098], 'LabelHorizontalAlignment','left');
yline(rHi, ':', sprintf('%.2f V', rHi), 'Color',[0.85 0.325 0.098], 'LabelHorizontalAlignment','left');
grid on; box on;
xlabel('Time (\mus)'); ylabel('Voltage (V)');
title('Slope Compensation Waveform (2.5 V in, 5 \Omega load)');
xlim(zoomLim);
ylim([-0.1 1.15]);
text(0.02, 0.95, sprintf('f_{sw} = %.1f kHz  (T = %.1f \\mus)\nRamp \\approx %.2f V to %.2f V (%.0f mV)', ...
     fsw/1e3, Tsw, rLo, rHi, (rHi-rLo)*1e3), ...
     'Units','normalized', 'VerticalAlignment','top', 'FontSize', 10, ...
     'BackgroundColor','w', 'EdgeColor',[0.6 0.6 0.6]);
exportgraphics(figSC, fullfile(dataDir, 'slope_compensation.png'), 'Resolution', 300);

%% Summary table ----------------------------------------------------------
T = struct2table([res25, res35]');
disp(T);
writetable(T, fullfile(dataDir, 'closed_loop_summary.csv'));
fprintf('\nSlope compensation: f_sw = %.1f kHz, ramp %.2f-%.2f V\n', fsw/1e3, rLo, rHi);


%% ======================= Local functions ===============================

function res = buildGridFigure(dataDir, files, loadLabels, vinLabel, figName, pngFile)
% 3x3 grid for one input voltage. Rows: DC / ripple / rise time.
% Columns: one per load (files{row, col}).
    blue = [0 0.447 0.741];
    red  = [0.85 0.325 0.098];
    nL   = numel(loadLabels);

    % Shared y-limits per row so the columns can be compared directly.
    % Set useCommonY = false to let each panel autoscale to its own data.
    useCommonY = true;
    dcLim = [4 6.5];            % V
    acLim = [-500 500];         % mV
    trLim = [-0.5 7.5];         % V

    fig = figure('Name', figName, 'Color','w', 'Position',[20 40 1800 950]);
    tl  = tiledlayout(fig, 3, nL, 'TileSpacing','compact', 'Padding','compact');
    title(tl, ['Boost Converter Closed-Loop Testing (' vinLabel ' in, 5 V out)'], ...
          'FontWeight','bold');

    for c = 1:nL
        lbl = loadLabels{c};
        DC = readmatrix(fullfile(dataDir, files{1,c}), 'NumHeaderLines', 1);
        AC = readmatrix(fullfile(dataDir, files{2,c}), 'NumHeaderLines', 1);
        TR = readmatrix(fullfile(dataDir, files{3,c}), 'NumHeaderLines', 1);

        % ---- Row 1: steady state (DC) ----
        t = DC(:,1)*1e3;  v = DC(:,2);                 % ms, V
        vMean = mean(v);

        nexttile(c);
        plot(t, v, 'Color', blue, 'LineWidth', 0.8); hold on;
        yline(vMean, '--', sprintf('Mean = %.3f V', vMean), 'Color', red, ...
              'LineWidth', 1.5, 'LabelHorizontalAlignment','left', ...
              'LabelVerticalAlignment','bottom', 'FontSize', 10);
        grid on; box on;
        xlabel('Time (ms)'); ylabel('Output voltage (V)');
        title([lbl ' - Steady State (DC coupled)']);
        xlim([min(t) max(t)]);
        if useCommonY, ylim(dcLim); else, ylim([vMean-1, vMean+1]); end

        % ---- Row 2: voltage ripple (AC) ----
        t = AC(:,1)*1e3;  v = AC(:,2)*1e3;             % ms, mV
        vpp  = max(v) - min(v);                        % includes spikes
        vrms = sqrt(mean((v - mean(v)).^2));
        clipPct = 1;                                   % ignore outer 1% each end (spikes)
        [vLo, vHi] = bandEdges(v, clipPct);
        vRipple = vHi - vLo;

        nexttile(nL + c);
        plot(t, v, 'Color', blue, 'LineWidth', 0.8); hold on;
        yline(vHi, '--', 'Color', red, 'LineWidth', 1.5);
        yline(vLo, '--', 'Color', red, 'LineWidth', 1.5);
        grid on; box on;
        xlabel('Time (ms)'); ylabel('Ripple voltage (mV)');
        title([lbl ' - Output Ripple (AC coupled)']);
        xlim([min(t) max(t)]);
        if useCommonY, ylim(acLim); end
        text(0.02, 0.97, sprintf(['Ripple (band) = %.0f mV\n' ...
                                  'V_{pk-pk} = %.0f mV\n' ...
                                  'V_{RMS} = %.1f mV'], vRipple, vpp, vrms), ...
             'Units','normalized', 'VerticalAlignment','top', 'FontSize', 9, ...
             'BackgroundColor','w', 'EdgeColor',[0.6 0.6 0.6]);

        % ---- Row 3: rise time (TR) ----
        % Baseline = first 20% of the capture, final value = last 20%, so this
        % works whatever time span the scope captured.
        t = TR(:,1);  v = TR(:,2);                     % s, V
        span   = max(t) - min(t);
        vBase  = mean(v(t < min(t) + 0.2*span));
        vFinal = mean(v(t > max(t) - 0.2*span));
        vs     = movmean(v, 200);                      % smoothed copy for edge detection
        lvl10  = vBase + 0.1*(vFinal - vBase);
        lvl90  = vBase + 0.9*(vFinal - vBase);
        t10 = crossTime(vs, t, lvl10);
        t90 = crossTime(vs, t, lvl90);
        tr  = t90 - t10;
        overshoot = 100*(max(vs) - vFinal)/(vFinal - vBase);

        nexttile(2*nL + c);
        plot(t*1e3, v, 'Color', blue, 'LineWidth', 0.8); hold on;
        yline(vFinal, ':', sprintf('Final = %.2f V', vFinal), 'Color',[0.4 0.4 0.4], ...
              'LabelHorizontalAlignment','right', 'LabelVerticalAlignment','bottom');
        plot([t10 t90]*1e3, [lvl10 lvl90], 'o', 'Color', red, ...
             'MarkerFaceColor', red, 'MarkerSize', 6);
        xline(t10*1e3, '--', '10%', 'Color', red, 'LabelOrientation','horizontal');
        xline(t90*1e3, '--', '90%', 'Color', red, 'LabelOrientation','horizontal');
        grid on; box on;
        xlabel('Time (ms)'); ylabel('Output voltage (V)');
        title(sprintf('%s - Rise Time (10%%-90%%): %.1f ms', lbl, tr*1e3));
        xlim([max(-200, min(t)*1e3), min(1000, max(t)*1e3)]);   % zoom on the transition
        if useCommonY, ylim(trLim); end

        res(c) = struct('name', sprintf('%s, %s', vinLabel, strrep(lbl, '\Omega', 'ohm')), ...
                        'dcMean_V', vMean, ...
                        'rippleBand_mV', vRipple, ...
                        'rippleRMS_mV', vrms, ...
                        'ripplePkPk_mV', vpp, ...
                        'riseTime_ms', tr*1e3, ...
                        'overshoot_pct', overshoot); %#ok<AGROW>
    end

    exportgraphics(fig, pngFile, 'Resolution', 300);
end

function [lo, hi] = bandEdges(x, pct)
% Lower/upper edges after discarding the outer pct% of samples at each end.
    s  = sort(x(:));
    n  = numel(s);
    lo = s(max(1, round(pct/100*n)));
    hi = s(min(n, round((100-pct)/100*n)));
end

function tc = crossTime(vs, t, lvl)
% First time the (smoothed) signal reaches lvl, linearly interpolated.
    i  = find(vs >= lvl, 1);
    tc = interp1(vs(i-1:i), t(i-1:i), lvl);
end