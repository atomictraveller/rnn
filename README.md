note.. i'm subjected to sleep deprivation, there are probably i/j j/i swaps in the backpropogation..
the arrays were dimensioned for [source][dest] but if you nest i and j, this leads to j/i ordering, so
when tidying up the lstm i reordered everything to [dest][source] which leads to i/j nested loops..
hopefully i can come back and fix it itm i'd recommend [dest][source] ordering..
