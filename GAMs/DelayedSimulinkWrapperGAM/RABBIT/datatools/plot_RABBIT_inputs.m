function plot_RABBIT_inputs(params,signals,t)
% plot_RABBIT_inputs(params,signals,t)

R = params.Value.R;
Z = params.Value.Z;

ttt=signals.equil.Psi.Time;
it = find(ttt>=t,1,'first');
assert(~isempty(it),'time index out of range [%3.3f,%3.3f]',min(ttt),max(ttt));
time = signals.equil.Psi.Time(it);

Psi = reshape(signals.equil.Psi.getdatasamples(it),numel(R),numel(Z))';
Psia = signals.equil.Psi_axis.getdatasamples(it);
Psib = signals.equil.Psi_sep.getdatasamples(it);
iota = signals.equil.iota.getdatasamples(it);
rhotor = signals.equil.rhotor1d.getdatasamples(it);
psi1d_n = signals.equil.psi1d_n.getdatasamples(it);
F = signals.equil.F.getdatasamples(it);
Volume = signals.equil.Volume.getdatasamples(it);
Area = signals.equil.Area.getdatasamples(it);

rhotork = signals.profiles.rhotor_kinprof.getdatasamples(it);
Te = signals.profiles.Te.getdatasamples(it);
Ti = signals.profiles.Ti.getdatasamples(it);
ne = signals.profiles.ne.getdatasamples(it);
Zeff = signals.profiles.Zeff.getdatasamples(it);
omega_tor = signals.profiles.omega_tor.getdatasamples(it);


PsiN = (Psi-Psia)/(Psib-Psia);

%%
nc = 6;

hf=figure(1); clf(hf); set(hf,'position',[100 100 1000 400])
ax = subplot(1,nc,1);
contourf(ax,R,Z,PsiN,linspace(0,2,21)); hold(ax,'on')
contour(ax,R,Z,PsiN,[1 1],'w','linewidth',2);
title(ax,sprintf('Equilibrium at t=%3.3fs',time));
colorbar(ax,'location','southoutside');
axis(ax,'equal');

ax = subplot(2,nc,2);
plot(ax,rhotor,iota); title(ax,'iota')
ax = subplot(2,nc,3);
plot(ax,rhotor,psi1d_n);title(ax,'psi1dN')

ax = subplot(2,nc,4);
plot(ax,rhotor,F);title(ax,'F=RBphi')

ax = subplot(2,nc,5);
plot(ax,rhotor,Volume);title(ax,'Volume')

ax = subplot(2,nc,6);
plot(ax,rhotor,Area);title(ax,'Area')


ax = subplot(2,nc,8);
plot(ax,rhotork,[Te;Ti]);title(ax,'Te,Ti')

ax = subplot(2,nc,9);
plot(ax,rhotork,ne);title(ax,'ne')

ax = subplot(2,nc,10);
plot(ax,rhotork,Zeff);title(ax,'Zeff')

ax = subplot(2,nc,11);
plot(ax,rhotork,omega_tor);title(ax,'omega_tor')

