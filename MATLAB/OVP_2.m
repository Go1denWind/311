%% OVP_PWM_analysis.m
% Boost converter bench captures: OVP test + PWM chain (PWM -> RC filter, duty out)
% Figures produced:
%   1  OVP test (full capture + zoom on the event)
%   2  PWM       - ATmega generated PWM
%   3  PWM_FILT  - PWM after the RC filter
%   4  DUTY      - gate drive into the boost MOSFET
%   5  Overlay of PWM / PWM_FILT / DUTY on a common time axis
clear; clc; close all;

%% ---- Settings (edit these) ----
dataDir  = 'C:\Users\tytom\OneDrive\Documents\GitHub\311\Oscilloscope\Output Generator\Closed Loop Testing';
outDir   = dataDir;      % where the PNG figures are saved

ovpFile  = fullfile(dataDir, 'OVP_5V5.CSV');
pwmFile  = fullfile(dataDir, 'PWM2.CSV');
filtFile = fullfile(dataDir, 'PWM_FILT.CSV');
dutyFile = fullfile(dataDir, 'DUTY2.CSV');

Vin      = 3.5;      % input voltage [V]   (PWM/duty captures)
Rload    = 10;       % load resistance [ohm]
Vthresh  = 5.5;      % OVP threshold line [V]
smoothN  = 101;      % smoothing window for OVP trace (samples)
ovpZoom  = [-0.2 1.0];   % OVP zoom window [ms]
pwmZoom  = [-10 20];     % PWM zoom window [us]  (about 3 cycles at 100 kHz)
saveFigs = true;         % save every figure as a PNG

condStr = sprintf('V_{in} = %.1f V, %g \\Omega load', Vin, Rload);

%% =====================================================================
%  1) OVP TEST
%  =====================================================================
[t, v] = loadScope(ovpFile);
vs  = movmean(v, smoothN);
tms = t * 1e3;

Vbase  = median(v(t < -0.5e-3));
Vfinal = mean(v(t > 0.8*t(end)));
[Vpk, iPk] = max(v);
overshoot  = Vpk - Vfinal;

iStart = find(vs > Vbase + 0.5, 1, 'first');
lvl10  = Vbase + 0.1*(Vfinal - Vbase);
lvl90  = Vbase + 0.9*(Vfinal - Vbase);
i10    = find(vs >= lvl10, 1, 'first');
i90    = find(vs >= lvl90, 1, 'first');
tRise  = t(i90) - t(i10);
iThr   = find(vs >= Vthresh, 1, 'first');

fprintf('===== OVP test =====\n');
fprintf('Baseline level     : %.3f V\n', Vbase);
fprintf('Settled level      : %.3f V\n', Vfinal);
fprintf('Peak voltage       : %.3f V at t = %.1f us\n', Vpk, t(iPk)*1e6);
fprintf('Overshoot vs final : %.3f V\n', overshoot);
fprintf('Event start        : %.1f us\n', t(iStart)*1e6);
fprintf('10-90%% rise time   : %.1f us\n', tRise*1e6);
if ~isempty(iThr)
    fprintf('Threshold (%.2f V) crossed at t = %.1f us\n', Vthresh, t(iThr)*1e6);
end

figure(1); set(gcf, 'Color', 'w', 'Position', [100 100 1000 750]);

ax1 = subplot(2,1,1); hold on; grid on; box on;
plot(tms, v,  'Color', [0.75 0.85 1.0], 'LineWidth', 0.5, 'DisplayName', 'Raw');
plot(tms, vs, 'Color', [0 0.25 0.75],   'LineWidth', 1.5, 'DisplayName', 'Smoothed');
yline(Vthresh, '--r', sprintf('OVP threshold = %.1f V', Vthresh), ...
      'LineWidth', 1.5, 'LabelHorizontalAlignment', 'left', 'HandleVisibility', 'off');
plot(tms(iPk), Vpk, 'kv', 'MarkerFaceColor', 'k', ...
     'DisplayName', sprintf('Peak = %.2f V', Vpk));
xlabel('Time (ms)'); ylabel('Voltage (V)');
title('Over-Voltage Protection Test - Full Capture');
legend('Location', 'southeast');
xlim([tms(1) tms(end)]);

ax2 = subplot(2,1,2); hold on; grid on; box on;
plot(tms, v,  'Color', [0.75 0.85 1.0], 'LineWidth', 0.5);
plot(tms, vs, 'Color', [0 0.25 0.75],   'LineWidth', 1.5);
yline(Vthresh, '--r', 'LineWidth', 1.5);
yline(Vbase,  ':k', sprintf('Baseline %.2f V', Vbase), 'LabelHorizontalAlignment', 'left');
yline(Vfinal, ':k', sprintf('Settled %.2f V', Vfinal), 'LabelHorizontalAlignment', 'left');
if ~isempty(iThr)
    xline(t(iThr)*1e3, '--', sprintf('Crosses threshold: %.0f us', t(iThr)*1e6), ...
          'Color', [0.8 0 0], 'LabelVerticalAlignment', 'bottom');
end
xlabel('Time (ms)'); ylabel('Voltage (V)');
title(sprintf('Zoom on Event  (10-90%% rise time = %.0f \\mus)', tRise*1e6));
xlim(ovpZoom);
linkaxes([ax1 ax2], 'y');

styleAx([ax1 ax2]);
lgd = legend(ax1); lgd.TextColor = 'k'; lgd.Color = 'w'; lgd.EdgeColor = 'k';

%% =====================================================================
%  2) PWM CHAIN  (PWM, PWM_FILT, DUTY)
%  =====================================================================
[tP, vP] = loadScope(pwmFile);
[tF, vF] = loadScope(filtFile);
[tD, vD] = loadScope(dutyFile);

mP = pwmStats(tP, vP);
mD = pwmStats(tD, vD);

sF       = sort(vF); nF = numel(sF);
VF_mean  = mean(vF);
VF_hi    = sF(round(0.99*nF));                                % robust upper ripple bound
VF_lo    = sF(max(1, round(0.01*nF)));                        % robust lower ripple bound
VF_pp    = VF_hi - VF_lo;                                     % ripple p-p

fprintf('\n===== PWM chain  (%s) =====\n', strrep(condStr, '\', ''));
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

colPWM  = [0.00 0.35 0.75];
colFilt = [0.85 0.33 0.10];
colDuty = [0.10 0.55 0.20];

% Figure 2: PWM
plotSignal(2, tP, vP, ['PWM (ATmega PWM) - ' condStr], colPWM, pwmZoom, ...
    { mP.vhigh, sprintf('High %.2f V', mP.vhigh), ':k'; ...
      mP.vlow,  sprintf('Low %.2f V',  mP.vlow),  ':k' }, ...
    dutyMarker(mP, pwmZoom, sprintf('Duty = %.1f %%', mP.duty*100)), []);

% Figure 3: PWM_FILT  (horizontal cursors: mean + ripple bounds)
plotSignal(3, tF, vF, ['PWM after RC filter - ' condStr], colFilt, pwmZoom, ...
    { VF_mean, sprintf('Mean = %.3f V', VF_mean), '-k'; ...
      VF_hi,   sprintf('Max = %.3f V',  VF_hi),   '--k'; ...
      VF_lo,   sprintf('Min = %.3f V',  VF_lo),   '--k' }, ...
    [], struct('lo', VF_lo, 'hi', VF_hi, ...
               'label', sprintf('Ripple = %.0f mV p-p', VF_pp*1e3)));

% Figure 4: DUTY
plotSignal(4, tD, vD, ['DUTY (MOSFET gate drive) - ' condStr], colDuty, pwmZoom, ...
    { mD.vhigh, sprintf('High %.2f V', mD.vhigh), ':k'; ...
      mD.vlow,  sprintf('Low %.2f V',  mD.vlow),  ':k' }, ...
    dutyMarker(mD, pwmZoom, sprintf('Duty = %.1f %%', mD.duty*100)), []);

% Figure 5: overlay (stacked, shared time axis)
figure(5); set(gcf, 'Color', 'w', 'Position', [100 100 1000 800]);
a1 = subplot(3,1,1); plot(tP*1e6, vP, 'Color', colPWM,  'LineWidth', 1.2);
ylabel('PWM (V)');     title(['Signal Chain Comparison - ' condStr]);
a2 = subplot(3,1,2); plot(tF*1e6, vF, 'Color', colFilt, 'LineWidth', 1.2);
ylabel('PWM filtered (V)');
a3 = subplot(3,1,3); plot(tD*1e6, vD, 'Color', colDuty, 'LineWidth', 1.2);
ylabel('DUTY (V)');    xlabel('Time (\mus)');
for a = [a1 a2 a3]
    grid(a, 'on'); box(a, 'on'); xlim(a, pwmZoom);
end
linkaxes([a1 a2 a3], 'x');
styleAx([a1 a2 a3]);

%% ---- Save figures ----
if saveFigs
    names = {'OVP_plot', 'PWM_plot', 'PWM_FILT_plot', 'DUTY_plot', 'signal_chain_plot'};
    for k = 1:5
        exportgraphics(figure(k), fullfile(outDir, [names{k} '.png']), 'Resolution', 200);
    end
end

%% =====================================================================
%  Local functions
%  =====================================================================
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