%% OVP_PWM_analysis.m
% Boost converter bench captures: OVP test + PWM chain (PWM -> RC filter, duty out)
% Figures produced:
%   1      OVP test (full capture + zoom on the rising edge with 10-90 % rise-time cursors)
%   2-4    PWM / PWM_FILT / DUTY at the first input voltage in 'sets'  (3.5 V)
%   5-7    PWM / PWM_FILT / DUTY at the second input voltage in 'sets' (2.5 V)
%   8      Comparison: PWM / PWM_FILT / DUTY with every input voltage on the same axes
%          (one extra trio of figures is made for each entry added to 'sets')
clear; clc; close all;

%% ---- Settings (edit these) ----
dataDir  = 'C:\Users\tytom\Documents\GitHub\311\Oscilloscope\Output Generator\Closed Loop Testing';
outDir   = dataDir;      % where the PNG figures are saved

ovpFile  = fullfile(dataDir, 'OVP.CSV');
% PWM-chain captures: one entry per input voltage (same load for all)
sets = struct( ...
    'Vin',  {3.5, 2.5}, ...
    'pwm',  {fullfile(dataDir, 'PWM2.CSV'),     fullfile(dataDir, 'PWM2V5.CSV')}, ...
    'filt', {fullfile(dataDir, 'PWM_FILT.CSV'), fullfile(dataDir, 'PWMF2V5.CSV')}, ...
    'duty', {fullfile(dataDir, 'DUTY2.CSV'),    fullfile(dataDir, 'DUTY2V5.CSV')});
Rload    = 10;       % load resistance [ohm]
ovpSmoothN    = 11;      % smoothing window for the OVP trace (samples)
ovpZoomMargin = 1.0;     % OVP zoom = rising edge +/- (this x the rise time) either side
pwmZoom  = [-10 20];     % PWM zoom window [us]  (about 3 cycles at 100 kHz)
saveFigs = true;         % save every figure as a PNG

%% =====================================================================
%  1) OVP TEST   (time axis shifted so the capture starts at 0 ns)
%  =====================================================================
[t, v] = loadScope(ovpFile);
tns = (t - t(1)) * 1e9;                    % time [ns], relative to first sample
vs  = movmean(v, ovpSmoothN);              % lightly smoothed copy
nS  = numel(v);

Vbase  = median(v(1:round(0.05*nS)));      % pre-event level (first 5 % of samples)
Vfinal = mean(v(round(0.8*nS):end));       % settled level   (last 20 % of samples)
[Vpk, iPk] = max(v);
overshoot  = Vpk - Vfinal;

lvl10 = Vbase + 0.1*(Vfinal - Vbase);
lvl90 = Vbase + 0.9*(Vfinal - Vbase);
t10   = crossTime(tns, vs, lvl10);         % time the edge crosses 10 % [ns]
t90   = crossTime(tns, vs, lvl90);         % time the edge crosses 90 % [ns]
tRise = t90 - t10;                         % 10-90 % rise time [ns]

fprintf('===== OVP test =====\n');
fprintf('Baseline level     : %.3f V\n', Vbase);
fprintf('Settled level      : %.3f V\n', Vfinal);
fprintf('Peak voltage       : %.3f V at t = %.1f ns\n', Vpk, tns(iPk));
fprintf('Overshoot vs final : %.3f V\n', overshoot);
fprintf('10%% crossing       : %.1f ns\n', t10);
fprintf('90%% crossing       : %.1f ns\n', t90);
fprintf('10-90%% rise time   : %.1f ns\n', tRise);

rawCol = [0.75 0.85 1.0];
smCol  = [0 0.25 0.75];
yArrow = 0.5 * (Vbase + lvl10);            % height of the rise-time dimension arrow

figure(1); set(gcf, 'Color', 'w', 'Position', [100 100 1000 750]);

% Full capture
ax1 = subplot(2,1,1); hold on; grid on; box on;
plot(tns, v,  'Color', rawCol, 'LineWidth', 0.5, 'DisplayName', 'Raw');
plot(tns, vs, 'Color', smCol,  'LineWidth', 1.5, 'DisplayName', 'Smoothed');
plot(tns(iPk), Vpk, 'kv', 'MarkerFaceColor', 'k', ...
     'DisplayName', sprintf('Peak = %.2f V', Vpk));
xlabel('Time (ns)'); ylabel('Voltage (V)');
title('Over-Voltage Protection Test - Full Capture');
xlim([tns(1) tns(end)]);
addRiseCursors(t10, t90, tRise, yArrow, false);
legend('Location', 'southeast');

% Zoom on the rising edge
ax2 = subplot(2,1,2); hold on; grid on; box on;
plot(tns, v,  'Color', rawCol, 'LineWidth', 0.5);
plot(tns, vs, 'Color', smCol,  'LineWidth', 1.5);
yline(Vbase,  ':k', sprintf('Baseline %.2f V', Vbase), ...
      'LabelHorizontalAlignment', 'left', 'FontWeight', 'bold');
yline(Vfinal, ':k', sprintf('Settled %.2f V', Vfinal), ...
      'LabelHorizontalAlignment', 'left', 'FontWeight', 'bold');
xlabel('Time (ns)'); ylabel('Voltage (V)');
title(sprintf('Zoom on Rising Edge  (10-90%% rise time = %.0f ns)', tRise));
xlim([max(tns(1),   t10 - ovpZoomMargin*tRise), ...
      min(tns(end), t90 + ovpZoomMargin*tRise)]);
addRiseCursors(t10, t90, tRise, yArrow, true);
linkaxes([ax1 ax2], 'y');

styleAx([ax1 ax2]);
lgd = legend(ax1); lgd.TextColor = 'k'; lgd.Color = 'w'; lgd.EdgeColor = 'k';

%% =====================================================================
%  2) PWM CHAIN  (PWM, PWM_FILT, DUTY) at each input voltage
%  =====================================================================
colPWM  = [0.00 0.35 0.75];
colFilt = [0.85 0.33 0.10];
colDuty = [0.10 0.55 0.20];

nSets = numel(sets);
for k = 1:nSets
    Vin     = sets(k).Vin;
    condStr = sprintf('V_{in} = %.1f V, %g \\Omega load', Vin, Rload);
    fig0    = 2 + 3*(k-1);                     % first figure number for this input voltage

    [tP, vP] = loadScope(sets(k).pwm);
    [tF, vF] = loadScope(sets(k).filt);
    [tD, vD] = loadScope(sets(k).duty);

    mP = pwmStats(tP, vP);
    mD = pwmStats(tD, vD);

    sF       = sort(vF); nF = numel(sF);
    VF_mean  = mean(vF);
    VF_hi    = sF(round(0.99*nF));                            % robust upper ripple bound
    VF_lo    = sF(max(1, round(0.01*nF)));                    % robust lower ripple bound
    VF_pp    = VF_hi - VF_lo;                                 % ripple p-p

    fprintf('\n===== PWM chain  (Vin = %.1f V, %g ohm load) =====\n', Vin, Rload);
    fprintf('PWM      : f = %.2f kHz, duty(high) = %.1f %%, high = %.2f V, low = %.2f V, mean = %.3f V\n', ...
            mP.freq/1e3, mP.duty*100, mP.vhigh, mP.vlow, mean(vP));
    fprintf('PWM_FILT : mean = %.3f V, ripple p-p = %.0f mV\n', VF_mean, VF_pp*1e3);
    fprintf('DUTY     : f = %.2f kHz, duty = %.1f %%, high = %.2f V, low = %.2f V\n', ...
            mD.freq/1e3, mD.duty*100, mD.vhigh, mD.vlow);
    fprintf('Check    : mean(PWM) = %.3f V vs PWM_FILT mean = %.3f V\n', mean(vP), VF_mean);
    % DUTY is the gate drive straight into the boost MOSFET, so its high-time is the on-time duty D
    Don = mD.duty;
    fprintf('Gate duty D = %.1f %%  ->  ideal boost Vout = Vin/(1-D) = %.2f V\n', ...
            Don*100, Vin/(1-Don));

    % PWM
    plotSignal(fig0, tP, vP, ['PWM (ATmega PWM) - ' condStr], colPWM, pwmZoom, ...
        { mP.vhigh, sprintf('High %.2f V', mP.vhigh), ':k'; ...
          mP.vlow,  sprintf('Low %.2f V',  mP.vlow),  ':k' }, ...
        dutyMarker(mP, pwmZoom, sprintf('Duty = %.1f %%', mP.duty*100)), []);

    % PWM_FILT  (horizontal cursors: mean + ripple bounds)
    plotSignal(fig0+1, tF, vF, ['PWM after RC filter - ' condStr], colFilt, pwmZoom, ...
        { VF_mean, sprintf('Mean = %.3f V', VF_mean), '-k'; ...
          VF_hi,   sprintf('Max = %.3f V',  VF_hi),   '--k'; ...
          VF_lo,   sprintf('Min = %.3f V',  VF_lo),   '--k' }, ...
        [], struct('lo', VF_lo, 'hi', VF_hi, ...
                   'label', sprintf('Ripple = %.0f mV p-p', VF_pp*1e3)));

    % DUTY
    plotSignal(fig0+2, tD, vD, ['DUTY (MOSFET gate drive) - ' condStr], colDuty, pwmZoom, ...
        { mD.vhigh, sprintf('High %.2f V', mD.vhigh), ':k'; ...
          mD.vlow,  sprintf('Low %.2f V',  mD.vlow),  ':k' }, ...
        dutyMarker(mD, pwmZoom, sprintf('Duty = %.1f %%', mD.duty*100)), []);

    % keep the waveforms for the comparison figure
    S(k).Vin = Vin;
    S(k).tP = tP;  S(k).vP = vP;
    S(k).tF = tF;  S(k).vF = vF;
    S(k).tD = tD;  S(k).vD = vD;
end

%% =====================================================================
%  3) COMPARISON  - every input voltage on the same axes
%  =====================================================================
figOv  = 2 + 3*nSets;
setCol = [0.00 0.25 0.75;     % 1st input voltage  (blue)
          0.85 0.15 0.10;     % 2nd input voltage  (red)
          0.10 0.55 0.20;     % 3rd (if you add more sets)
          0.50 0.20 0.65];
sigKey  = {'P', 'F', 'D'};
sigName = {'PWM (V)', 'PWM filtered (V)', 'DUTY (V)'};

figure(figOv); set(gcf, 'Color', 'w', 'Position', [100 100 1000 800]);
axOv = gobjects(1, 3);
for r = 1:3
    axOv(r) = subplot(3,1,r); hold on; grid on; box on;
    for k = 1:nSets
        plot(S(k).(['t' sigKey{r}]) * 1e6, S(k).(['v' sigKey{r}]), ...
             'Color', setCol(k,:), 'LineWidth', 1.2, ...
             'DisplayName', sprintf('V_{in} = %.1f V', S(k).Vin));
    end
    ylabel(sigName{r});
    xlim(pwmZoom);
end
xlabel(axOv(3), 'Time (\mus)');
vinList = strjoin(compose('%.1f V', [S.Vin]), ' vs ');
title(axOv(1), sprintf('Signal Chain Comparison - %s V_{in}, %g \\Omega load', vinList, Rload));
linkaxes(axOv, 'x');
styleAx(axOv);
for r = 1:3
    lg = legend(axOv(r), 'Location', 'best');
    lg.TextColor = 'k'; lg.Color = 'w'; lg.EdgeColor = 'k';
end

%% ---- Save figures ----
if saveFigs
    names = {'OVP_plot'};
    for k = 1:nSets
        tag   = strrep(sprintf('%gV', sets(k).Vin), '.', 'p');     % e.g. 3p5V
        names = [names, {['PWM_plot_' tag], ['PWM_FILT_plot_' tag], ['DUTY_plot_' tag]}]; %#ok<AGROW>
    end
    names{end+1} = 'signal_chain_comparison';
    for k = 1:numel(names)
        exportgraphics(figure(k), fullfile(outDir, [names{k} '.png']), 'Resolution', 200);
    end
end

%% =====================================================================
%  Local functions
%  =====================================================================
function tc = crossTime(t, v, lvl)
    % Time at which v first reaches lvl (linear interpolation between samples)
    i = find(v >= lvl, 1, 'first');
    if isempty(i),  tc = NaN;  return;  end
    if i == 1,      tc = t(1); return;  end
    tc = interp1(v(i-1:i), t(i-1:i), lvl);
end

function addRiseCursors(t10, t90, tRise, yArrow, showDim)
    % Vertical cursors at the 10 % and 90 % crossings (+ dimension arrow if showDim)
    xline(t10, '-k', '10 %', 'LineWidth', 1.2, 'FontWeight', 'bold', ...
          'LabelHorizontalAlignment', 'left', 'LabelVerticalAlignment', 'top', ...
          'HandleVisibility', 'off');
    xline(t90, '-k', '90 %', 'LineWidth', 1.2, 'FontWeight', 'bold', ...
          'LabelHorizontalAlignment', 'left', 'LabelVerticalAlignment', 'top', ...
          'HandleVisibility', 'off');
    if showDim
        xl = xlim;
        plot([t10 t90], [yArrow yArrow], '-k', 'LineWidth', 1.2);
        plot(t10, yArrow, '<k', 'MarkerFaceColor', 'k', 'MarkerSize', 6);
        plot(t90, yArrow, '>k', 'MarkerFaceColor', 'k', 'MarkerSize', 6);
        text(t90 + 0.02*diff(xl), yArrow, ...
             sprintf('10-90%% rise time = %.0f ns', tRise), ...
             'HorizontalAlignment', 'left', 'VerticalAlignment', 'middle', ...
             'FontWeight', 'bold', 'FontSize', 12, 'Color', 'k', ...
             'BackgroundColor', 'w', 'EdgeColor', 'k', 'Margin', 3);
    end
end

function [t, v] = loadScope(fname)
    % Scope CSV: col 1 = time [s], col 2 = voltage [V]
    T = readtable(fname, 'VariableNamingRule', 'preserve');
    t = T{:,1};
    v = T{:,2};
end

function m = pwmStats(t, v)
    % Frequency, duty (fraction of time HIGH) and levels of a PWM waveform
    sv = sort(v); n = numel(sv);
    lo = sv(max(1, round(0.02*n)));
    hi = sv(round(0.98*n));
    mid   = (lo + hi) / 2;
    above = movmean(v, 25) > mid;                       % lightly smoothed threshold
    rises = find(~above(1:end-1) & above(2:end));
    falls = find(above(1:end-1) & ~above(2:end));
    m.tRise = t(rises);                                 % rising-edge times [s]
    m.tFall = t(falls);                                 % falling-edge times [s]
    m.freq  = 1 / mean(diff(t(rises)));
    m.duty  = mean(above(rises(1):rises(end)-1));       % whole cycles only
    m.vhigh = median(v(above));
    m.vlow  = median(v(~above));
end

function plotSignal(figNum, t, v, ttl, col, zoomUs, hl, duty, rip)
    % Full capture (top) + zoom on a few cycles (bottom), time in us
    %   hl   : Nx3 cell {y, label, lineSpec} horizontal cursors
    %   duty : struct from dutyMarker (vertical duty cursors), or []
    %   rip  : struct with lo, hi, label (ripple arrow), or []
    tus = t * 1e6;
    figure(figNum); set(gcf, 'Color', 'w', 'Position', [100 100 1000 700]);

    ax1 = subplot(2,1,1); hold on; grid on; box on;
    plot(tus, v, 'Color', col, 'LineWidth', 1);
    addLines(hl);
    xlabel('Time (\mus)'); ylabel('Voltage (V)');
    title([ttl ' - Full Capture']);
    xlim([tus(1) tus(end)]);

    ax2 = subplot(2,1,2); hold on; grid on; box on;
    plot(tus, v, 'Color', col, 'LineWidth', 1.3);
    addLines(hl);
    if ~isempty(duty), addDuty(duty); end
    if ~isempty(rip),  addRipple(rip, zoomUs); end
    xlabel('Time (\mus)'); ylabel('Voltage (V)');
    title('Zoom (a few cycles)');
    xlim(zoomUs);

    linkaxes([ax1 ax2], 'y');
    styleAx([ax1 ax2]);
end

function addLines(hl)
    % Horizontal cursors: {y, label, lineSpec}
    for k = 1:size(hl, 1)
        yline(hl{k,1}, hl{k,3}, hl{k,2}, 'LineWidth', 1.2, ...
              'LabelHorizontalAlignment', 'left', 'FontWeight', 'bold');
    end
end

function d = dutyMarker(m, zoomUs, label)
    % Pick one full cycle inside the zoom window: rising edge -> falling edge
    d  = [];
    tr = m.tRise * 1e6;   tf = m.tFall * 1e6;
    k  = find(tr(1:end-1) >= zoomUs(1) & tr(2:end) <= zoomUs(2), 1, 'first');
    if isempty(k), return; end
    j  = find(tf > tr(k), 1, 'first');
    if isempty(j) || tf(j) > tr(k+1), return; end
    d.x1 = tr(k);  d.x2 = tf(j);  d.xEnd = tr(k+1);
    d.y  = (m.vlow + m.vhigh) / 2;
    d.label = label;
end

function addDuty(d)
    % Vertical cursors at the pulse edges + dimension arrow with the duty value
    xline(d.x1, '-k', 'LineWidth', 1.2);
    xline(d.x2, '-k', 'LineWidth', 1.2);
    plot([d.x1 d.x2], [d.y d.y], '-k', 'LineWidth', 1.2);
    plot(d.x1, d.y, '<k', 'MarkerFaceColor', 'k', 'MarkerSize', 6);
    plot(d.x2, d.y, '>k', 'MarkerFaceColor', 'k', 'MarkerSize', 6);
    text((d.x1 + d.x2)/2, d.y, d.label, 'HorizontalAlignment', 'center', ...
         'VerticalAlignment', 'bottom', 'FontWeight', 'bold', 'FontSize', 12, ...
         'Color', 'k', 'BackgroundColor', 'w', 'Margin', 2);
end

function addRipple(r, zoomUs)
    % Vertical dimension arrow spanning the ripple bounds, labelled with p-p value
    x = zoomUs(2) - 0.06 * diff(zoomUs);
    plot([x x], [r.lo r.hi], '-k', 'LineWidth', 1.2);
    plot(x, r.hi, '^k', 'MarkerFaceColor', 'k', 'MarkerSize', 6);
    plot(x, r.lo, 'vk', 'MarkerFaceColor', 'k', 'MarkerSize', 6);
    text(x - 0.02 * diff(zoomUs), (r.lo + r.hi)/2, r.label, ...
         'HorizontalAlignment', 'right', 'VerticalAlignment', 'middle', ...
         'FontWeight', 'bold', 'FontSize', 12, 'Color', 'k', ...
         'BackgroundColor', 'w', 'EdgeColor', 'k', 'Margin', 3);
end

function styleAx(axs)
    % White axes, black text (overrides MATLAB's dark theme)
    for ax = axs
        ax.Color      = 'w';
        ax.XColor     = 'k';
        ax.YColor     = 'k';
        ax.GridColor  = 'k';
        ax.GridAlpha  = 0.15;
        ax.FontSize   = 12;
        ax.Title.Color      = 'k';
        ax.Title.FontWeight = 'bold';
        ax.XLabel.Color     = 'k';
        ax.YLabel.Color     = 'k';
    end
end