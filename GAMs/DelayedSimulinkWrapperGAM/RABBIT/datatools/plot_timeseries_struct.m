function plot_timeseries_struct(hf,tsstruct)

assert(isa(hf,'matlab.ui.Figure'),'hf must be figure handle');
assert(isstruct(tsstruct),'tsstruct must be a structure');
%%
fields = fieldnames(tsstruct);
nfields = numel(fields);

%%
% figure rows, columns
nc = floor(sqrt(nfields));
nr = ceil(nfields/nc);


for ii=1:nfields
    myfield = tsstruct.(fields{ii});
    assert(isa(myfield,'timeseries'));
    
    if strcmp(myfield.Name,'unnamed')
        myfield.Name = fields{ii};
    end
    
    hax=subplot(nr,nc,ii);
    plot(myfield);
    
end

end