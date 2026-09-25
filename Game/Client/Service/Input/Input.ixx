export module Client.Input;

//?!? payload를 적용해서 Runtime에서 Client 쪽으로 넘어올때 데이터를 전부 들고 오는 식으로 바꿔야 한다.
//?!? 모듈을 정리해서 바꾸는 것보다 Input을 아예 다시 만드는 쪽이 더 빠를듯.
export import Client.Input.Service;
export import Client.Input.Contract; //?!? Contract 전부를 export 하는게 아니라 필요한 것만 export 하게 수정해야 한다.