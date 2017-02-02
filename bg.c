#include <ncurses.h>
void init_fg(int c){
  for(int i=0;i<256;i++){
    init_pair(i,i,c);
  }
}
void init_bg(){
  for(int i=0;i<128;i++){
    init_pair(i,i,0);
  }
  for(int i=128;i<256;i++){
    init_pair(i,0,i);
  }
}

int main(){
  initscr();
  start_color();
  int i=0;
  timeout(1000);
  noecho();
  keypad(stdscr,TRUE);
  curs_set(2);
  int cursx = 0;
  int cursy = 0;
  if(!can_change_color()){
    mvaddstr(1,0,"no changies");
  }else{
    mvaddstr(1,0,"changies!");
  }
  init_fg(0);
    for(int i=0;i<16;i++){
      for(int j=0;j<16;j++){
	attron(COLOR_PAIR(i*16+j));
       	mvaddstr(i,j,"X");
	attroff(COLOR_PAIR(i*16+j));
      }
    }


  while(1){
    char str[20];
    int c = getch();
    if(c == KEY_LEFT && cursx > 0){
      cursx -= 1;
    }
    if(c == KEY_RIGHT && cursx < 16){
      cursx += 1;
    }
    if(c == KEY_UP && cursy > 0){
      cursy -= 1;
    }
    if(c == KEY_DOWN && cursy < 16){
      cursy += 1;
    }

      //    init_pair(1,0,cursx+16*cursy);
      //    bkgd(COLOR_PAIR(1));
      //    refresh();

    
    //    mvaddstr(18,0,0);
    sprintf(str,"Color %3d",cursy*16+cursx);
    init_fg(cursy*16+cursx);
    mvaddstr(18,0,str);
    move(cursy,cursx);
    refresh();	

  }
}
