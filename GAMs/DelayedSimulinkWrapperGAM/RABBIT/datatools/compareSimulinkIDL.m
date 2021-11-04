function errornorm = compareSimulinkIDL(data,idlsav,doplot)
% compares Simulilink output to stored IDL data from FORTRAN test run

idlfields = {'pheate','pheati','pshine','prot','ploss','pcx'};
simoutfields = {'powe_total','powi_total','Pshine','Prot','Porbitloss','Pcxloss'};

ibeam=3; % one beam only was used
sltime = data.time;
for ii = 1:numel(idlfields)
    idlf = idlfields{ii};
    simf = simoutfields{ii};
   
    if ii==1, 
        sldata = [];
        idldata = [];
        idltime = idlsav.beams{ibeam}.time;
        idlrho = idlsav.beams{ibeam}.rho;
    end
    sld = squeeze(data.(simf)(:,ibeam,:));
    idd = idlsav.beams{ibeam}.(idlf);
    sldata = [sldata,sld];
    idldata = [idldata,idd];
end

err = (idldata-sldata).^2 ./ idldata.^2;
err(isnan(err))=0;
errornorm = sqrt(sum(sum(err)))./size(err,2);

if doplot
clf;
hax(1)=subplot(221);
plot(sltime,cumsum(sldata'))
legend(simoutfields,'location','best','box','off')
title('Cumulative Simulink')

hax(2)=subplot(223);
plot(idltime,cumsum(idldata'))
legend(idlfields,'location','best','box','off')
title('Cumulative IDL')

hax(3)=subplot(222);
plot(idltime,idldata'); hold on;
plot(idltime,sldata','k--')
legend(idlfields,'location','best','box','off')
title('Compare (idl (-), simulink(--)')

hax(4)=subplot(224);
plot(sltime,100*sqrt(err));
title('relative errors [%]')
legend(idlfields,'location','best','box','off');

linkaxes(hax,'x');
shg

end

return