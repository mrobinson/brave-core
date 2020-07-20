declare namespace Playlists {
  export interface ApplicationState {
    playlistsData: State | undefined
  }

  export interface State {
    lists: []
  }
}
